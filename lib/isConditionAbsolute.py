import pyverilog.vparser.ast as vast
from pyverilog.vparser.parser import parse
from z3 import Solver, If, Int, Or, BitVecNumRef, And, Not, ZeroExt, BitVec, BitVecRef, unsat, BoolRef, Extract, BitVecVal
import sys


class isConditionAbsolute:
    def __init__(self):
        self.solver = Solver()
        self.var_map = {}
        self.targetIf = 0
        self.exit = False
        self.count = 0
        self.results = []

    def convert_ast_to_z3(self, node, negate):
        if self.exit:
            return True
        if isinstance(node, vast.IfStatement):
            if self.count == self.targetIf:
                cond = self.convert_expr_to_z3(node.cond)
                if not isinstance(cond, BoolRef):
                    cond = (cond != 0)
                if negate:
                    cond = Not(cond)
                self.exit = True
                self.results.append(cond)
                return cond
            self.count += 1
            cond = self.convert_expr_to_z3(node.cond)
            if not isinstance(cond, BoolRef):
                cond = (cond != 0)
            true_branch = self.convert_ast_to_z3(node.true_statement, negate)
            false_branch = self.convert_ast_to_z3(node.false_statement, negate) if node.false_statement else None
            if false_branch is not None:
                return If(cond, true_branch, false_branch)
            else:
                return If(cond, true_branch, True)
        if isinstance(node, vast.Always):
            sens_list = node.sens_list
            statements = []
            for sens in sens_list.list:
                if isinstance(sens, vast.Sens) and isinstance(sens.sig, (vast.Identifier, vast.Pointer)):
                    signal = self.convert_expr_to_z3(sens.sig)
                    if signal is not None:
                        if sens.type == 'posedge':
                            self.var_map[signal] = 1
                        elif sens.type == 'negedge':
                            self.var_map[signal] = 0
            if len(statements) != 0:
                return And(self.convert_ast_to_z3(node.statement, negate), Or(*statements) if len(statements) > 1 else statements[0])
            else:
                return self.convert_ast_to_z3(node.statement, negate)
        elif isinstance(node, (vast.Input, vast.Output, vast.Reg, vast.Wire)):
            width = abs(int(node.width.msb.value) - int(node.width.lsb.value)) + 1 if node.width else 1
            self.var_map[node.name] = BitVec(node.name, width)
            return True
        elif isinstance(node, (vast.BlockingSubstitution, vast.Assign, vast.NonblockingSubstitution)):
            self.convert_expr_to_z3(node)
            return True
        elif isinstance(node, vast.Block):
            exprs = []
            for stmt in node.statements:
                if stmt is not None and not self.exit:
                    expr = self.convert_ast_to_z3(stmt, negate)
                    exprs.append(expr)
                if self.exit:
                    break
            return And(*exprs) if len(exprs) > 1 else (exprs[0] if exprs else None)
        for child in node.children():
            child_expr = self.convert_ast_to_z3(child, negate)
            if child_expr is not None:
                self.results.append(child_expr)
            if self.exit:
                return self.results
        return None

    def convert_expr_to_z3(self, expr):
        if isinstance(expr, vast.Identifier):
            if expr.name not in self.var_map:
                print("variable undefined", file=sys.stderr)
                sys.exit(1)
            return self.var_map[expr.name]
        elif isinstance(expr, (vast.BlockingSubstitution, vast.Assign, vast.NonblockingSubstitution)):
            left, right = self.truncate_operands(expr.left.var, expr.right.var)
            self.var_map[expr.left.var.name] = right
        elif isinstance(expr, vast.Partselect):
            base_var = self.convert_expr_to_z3(expr.var)
            msb = max(self.convert_expr_to_z3(expr.msb), self.convert_expr_to_z3(expr.lsb))
            lsb = min(self.convert_expr_to_z3(expr.msb), self.convert_expr_to_z3(expr.lsb))
            return Extract(msb, lsb, base_var)
        elif isinstance(expr, vast.Pointer):
            base_var = self.convert_expr_to_z3(expr.var)
            index = self.convert_expr_to_z3(expr.ptr)
            return Extract(index, index, base_var)
        elif isinstance(expr, vast.IntConst):
            return int(expr.value)
        elif isinstance(expr, vast.LessThan):
            left, right = self.extend_operands(expr.left, expr.right)
            return left < right
        elif isinstance(expr, vast.GreaterThan):
            left, right = self.extend_operands(expr.left, expr.right)
            return left > right
        elif isinstance(expr, vast.LessEq):
            left, right = self.extend_operands(expr.left, expr.right)
            return left <= right
        elif isinstance(expr, vast.GreaterEq):
            left, right = self.extend_operands(expr.left, expr.right)
            return left >= right
        elif isinstance(expr, (vast.Eq, vast.Eql)):
            left, right = self.extend_operands(expr.left, expr.right)
            return left == right
        elif isinstance(expr, (vast.NotEq, vast.NotEql)):
            left, right = self.extend_operands(expr.left, expr.right)
            return left != right
        elif isinstance(expr, vast.Plus):
            left, right = self.extend_operands(expr.left, expr.right)
            return left + right
        elif isinstance(expr, vast.Minus):
            left, right = self.extend_operands(expr.left, expr.right)
            return left - right
        elif isinstance(expr, vast.Times):
            left, right = self.extend_operands(expr.left, expr.right)
            return left * right
        elif isinstance(expr, vast.Divide):
            left, right = self.extend_operands(expr.left, expr.right)
            return left / right
        elif isinstance(expr, vast.And):
            left, right = self.extend_operands(expr.left, expr.right)
            return left & right
        elif isinstance(expr, vast.Or):
            left, right = self.extend_operands(expr.left, expr.right)
            return left | right
        elif isinstance(expr, vast.Xor):
            left, right = self.extend_operands(expr.left, expr.right)
            return left ^ right
        elif isinstance(expr, vast.Unot):
            arg = self.convert_expr_to_z3(expr.right)
            return ~arg
        elif isinstance(expr, vast.Lor):
            left = self.convert_expr_to_z3(expr.left)
            right = self.convert_expr_to_z3(expr.right)
            if isinstance(left, (BitVecRef, BitVecNumRef)) or isinstance(left, int):
                left = (left != 0)
            if isinstance(right, (BitVecRef, BitVecNumRef)) or isinstance(right, int):
                right = (right != 0)
            return Or(left, right)
        elif isinstance(expr, vast.Land):
            left = self.convert_expr_to_z3(expr.left)
            right = self.convert_expr_to_z3(expr.right)
            if isinstance(left, (BitVecRef, BitVecNumRef)) or isinstance(left, int):
                left = (left != 0)
            if isinstance(right, (BitVecRef, BitVecNumRef)) or isinstance(right, int):
                right = (right != 0)
            return And(left, right)
        elif isinstance(expr, vast.Ulnot):
            arg = self.convert_expr_to_z3(expr.right)
            return Not(arg)
        elif isinstance(expr, vast.Cond):
            cond = self.convert_expr_to_z3(expr.cond)
            true_expr = self.convert_expr_to_z3(expr.true_value)
            false_expr = self.convert_expr_to_z3(expr.false_value)
            return If(cond, true_expr, false_expr)
        elif isinstance(expr, vast.Sll):
            left, right = self.extend_operands(expr.left, expr.right)
            return left << right
        elif isinstance(expr, vast.Srl):
            left, right = self.extend_operands(expr.left, expr.right)
            return left >> right
        return None

    def extend_operands(self, left_expr, right_expr):
        left = self.convert_expr_to_z3(left_expr)
        right = self.convert_expr_to_z3(right_expr)
        l = left.size() if isinstance(left, (BitVecRef, BitVecNumRef)) else 32
        r = right.size() if isinstance(right, (BitVecRef, BitVecNumRef)) else 32
        max_width = max(l, r)
        return self.extend_to_same_width(left, right, max_width)

    def truncate_operands(self, left_expr, right_expr):
        left = self.convert_expr_to_z3(left_expr)
        right = self.convert_expr_to_z3(right_expr)
        l = left.size() if isinstance(left, (BitVecRef, BitVecNumRef)) else 32
        r = right.size() if isinstance(right, (BitVecRef, BitVecNumRef)) else 32
        min_width = min(l, r)
        return self.truncate_to_same_width(left, right, min_width)

    def extend_to_same_width(self, left, right, max_width):
        if isinstance(left, int) and not isinstance(left, BitVecNumRef):
            left = BitVecVal(left, max_width)
        if isinstance(right, int) and not isinstance(right, BitVecNumRef):
            right = BitVecVal(right, max_width)
        lsize = left.size()
        rsize = right.size()
        left_ext = ZeroExt(max_width - lsize, left) if lsize < max_width else left
        right_ext = ZeroExt(max_width - rsize, right) if rsize < max_width else right
        return left_ext, right_ext

    def truncate_to_same_width(self, left, right, min_width):
        if isinstance(left, int) and not isinstance(left, (BitVecRef, BitVecNumRef)):
            left = BitVecVal(left, min_width)
        if isinstance(right, int) and not isinstance(right, (BitVecRef, BitVecNumRef)):
            right = BitVecVal(right, min_width)
        lsize = left.size()
        rsize = right.size()
        left_ext = Extract(min_width - 1, 0, left) if lsize > min_width else left
        right_ext = Extract(min_width - 1, 0, right) if rsize > min_width else right
        left_ext = left_ext if lsize >= min_width else ZeroExt(min_width - lsize, left)
        right_ext = right_ext if rsize >= min_width else ZeroExt(min_width - rsize, right)
        return left_ext, right_ext

    def analyze_verilog(self, verilog_file, target):
        self.targetIf = target
        ast, _ = parse([verilog_file], debug=False)
        for item in ast.description.definitions:
            exprs = self.convert_ast_to_z3(item, False)
        for e in exprs:
            self.solver.add(e)
        if (self.solver.check() == unsat):
            return -1
        else:
            self.solver.reset()
            self.var_map.clear()
            self.exit = False
            self.count = 0
            self.results.clear()
            for item in ast.description.definitions:
                exprs = self.convert_ast_to_z3(item, True)
            for e in exprs:
                self.solver.add(e)
            if (self.solver.check() == unsat):
                return 1
        return 0

# verilog_code = """
# module condition_program_second0(kdAF45, jeI, J, TC, i, b, Ct, j, v, M7);
# output [3:0] kdAF45;
# output jeI;
# reg jeI;
# output J;
# output TC;
# output [7:0] i;
# output b;
# output Ct;
# output j;
# input v;
# input M7;
# reg V;
# reg [6:0] F;
# reg J3;
# reg O1;
# reg CU0s;
# reg vX51JA8lr;
# reg G;
# reg r;
# reg u;
# reg pc7;
# reg TI;
# reg u40;
# always @ (posedge M7) begin
# r <= ~((M7) | ~((v)) | ~(((M7))) + v);
# if (M7) begin
# r <= 4;
# end
# end
# always @ (negedge M7) begin
# TI <= 126;
# F <= 80;
# if (!((M7) < (M7))) begin
# CU0s <= ~(2);
# if ((v) == (((~(~((~(77)))))))) begin
# if (!((CU0s) >= (M7) || v) || !((~(CU0s)) <= (56 | ~(CU0s))) && 2 + ~((CU0s) + (CU0s + (~(315) + F[5:3])) * M7) + ((CU0s)) && ~((v)) && !((F[6:1]) > (6))) begin
# u40 <= ~(~(F[5]) + (((~(F[3]))) & ~(0)));
# u <= (~(4 + 4));
# end
# else begin
# u40 <= ((CU0s));
# u <= (((~(~(51 | ~(763 - ~(CU0s))) & 4)))) | (30) - 42;
# end
# end
# else if (!((~(~(((((~(573)))))) | 6 - ~(~(v)))) <= (289))) begin
# TI <= (~(~(F[4])) + F[2]);
# u <= (((~((F[3])) - (2 * 8 + (v) & ~((~(v) - ~((7))))))));
# u40 <= ~(711);
# end
# else begin
# u <= ((TI) - M7) | 30;
# u40 <= ~((1) * v & 28);
# end
# J3 <= (v & CU0s);
# end
# else begin
# u40 <= 2;
# u <= 1;
# J3 <= (717);
# CU0s <= ~(v);
# end
# O1 <= 41;
# vX51JA8lr <= ~((62 - J3) - ~(~((v))) * (3) + (M7));
# end
# always @ (negedge M7) begin
# V <= ~(v + ~(~(v) * 1));
# if (!((~(M7) | 78 + (M7 & (486) + 334 | 41 & M7)) > (~(~(V)) | M7 | ~(v)))) begin
# G <= v;
# pc7 <= ((0)) | 74;
# end
# else begin
# pc7 <= ~(53);
# G <= M7;
# end
# end
# always @ (v or M7) begin
# jeI = v;
# end
# assign kdAF45 = 325;
# assign J = (pc7 + (r)) * u - 74 + G;
# assign TC = (~((331 - ~((~(46)) - ~((313))) & kdAF45 | (vX51JA8lr & 52 + ~((~(4 * F[1]))) & ((u40)) * 1 & 118))) + ~(42));
# assign i = O1;
# assign b = 721;
# assign Ct = TI;
# assign j = 88;
# endmodule
# """
#
# analyzer = isConditionAbsolute()
# print(analyzer.analyze_verilog(verilog_code, 1))
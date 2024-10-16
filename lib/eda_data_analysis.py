import pandas as pd
import statsmodels.api as sm
from sklearn.preprocessing import MinMaxScaler
from sklearn.model_selection import train_test_split
from sklearn.neural_network import MLPRegressor
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
import os
import pandas as pd
import matplotlib.pyplot as plt


def eda_data_analysis(filepath, output_dir, judge_bound, if_high_leverage=False, ifRoute=True):
    output_dir = os.path.abspath(output_dir)

    columns = ['test_name', 'test_seq', 'MR', 'main_blif', 'io_nums', 'blocks', 'total_nets', 'fanout_ave',
               'fanout_max', 'logic_level', 'prefer_ratio', 'wire_ratio', 'slack_delta', 'which_strange']
    if ifRoute:
        columns = ['test_name', 'test_seq', 'MR', 'main_blif', 'io_nums', 'blocks', 'total_nets', 'fanout_ave',
                   'fanout_max', 'logic_level', 'prefer_ratio', 'wire_ratio', 'slack_delta', 'route_prefer_ratio',
                   'route_wire_ratio', 'route_slack_delta', 'which_strange']

    target_analysis(filepath, output_dir, judge_bound, 'prefer_ratio', columns, if_high_leverage)
    target_analysis(filepath, output_dir, judge_bound, 'wire_ratio', columns, if_high_leverage)
    target_analysis(filepath, output_dir, judge_bound, 'slack_delta', columns, if_high_leverage)

    for mr_type in ['test_consistLayout', 'test_addRouteConsist']:
        pr_outliers_path = os.path.join(output_dir, f'duplicated_{mr_type}_prefer_ratio_outliers.csv')
        wr_outliers_path = os.path.join(output_dir, f'duplicated_{mr_type}_wire_ratio_outliers.csv')
        sl_outliers_path = os.path.join(output_dir, f'duplicated_{mr_type}_slack_delta_outliers.csv')

        if not os.path.exists(pr_outliers_path):
            pr_outliers = pd.DataFrame(
                columns=columns)
            pr_outliers.to_csv(pr_outliers_path, index=False)
        else:
            pr_outliers = pd.read_csv(pr_outliers_path)

        if not os.path.exists(wr_outliers_path):
            wr_outliers = pd.DataFrame(
                columns=columns)
            wr_outliers.to_csv(wr_outliers_path, index=False)
        else:
            wr_outliers = pd.read_csv(wr_outliers_path)

        if not os.path.exists(sl_outliers_path):
            sl_outliers = pd.DataFrame(
                columns=columns)
            sl_outliers.to_csv(sl_outliers_path, index=False)
        else:
            sl_outliers = pd.read_csv(sl_outliers_path)

        pr_outliers['which_strange'] = 'delay'
        wr_outliers['which_strange'] = 'wire'
        sl_outliers['which_strange'] = 'slack'
        combined_outliers = pd.concat([pr_outliers, wr_outliers])
        combined_outliers = pd.concat([combined_outliers, sl_outliers])

        if 'id' in combined_outliers.columns:
            combined_outliers.drop('id', axis=1, inplace=True)
        combined_outliers_grouped = combined_outliers.groupby(
            list(combined_outliers.columns.drop('which_strange'))).agg({
            'which_strange': lambda
                x: 'all' if 'delay' in x.values and 'wire' in x.values and 'slack' in x.values else (
                x.iloc[0] if len(x) > 0 else 'unknown')
        }).reset_index()

        if mr_type == 'test_consistLayout':
            combined_outliers_grouped = combined_outliers_grouped[
                ~(combined_outliers_grouped['prefer_ratio'] * combined_outliers_grouped['wire_ratio'] < 0)
            ]
        elif mr_type == 'test_addRouteConsist':
            combined_outliers_grouped = combined_outliers_grouped[
                (combined_outliers_grouped['prefer_ratio'] > 0) & (combined_outliers_grouped['wire_ratio'] > 0)
                & (combined_outliers_grouped['slack_delta'] < 0)
                ]

        final_path = os.path.join(output_dir, f"final_outliers_{mr_type}.csv")
        combined_outliers_grouped.to_csv(final_path, index=False)
        print(f"Final combined outliers saved to {final_path}")

    # router defect analysis
    if ifRoute:
        for mr_type in ['test_consistLayout', 'test_addRouteConsist']:
            pr_outliers_path = os.path.join(output_dir, f'duplicated_{mr_type}_route_prefer_ratio_outliers.csv')
            wr_outliers_path = os.path.join(output_dir, f'duplicated_{mr_type}_route_wire_ratio_outliers.csv')
            sl_outliers_path = os.path.join(output_dir, f'duplicated_{mr_type}_route_slack_delta_outliers.csv')

            if not os.path.exists(pr_outliers_path):
                pr_outliers = pd.DataFrame(
                    columns=columns)
                pr_outliers.to_csv(pr_outliers_path, index=False)
            else:
                pr_outliers = pd.read_csv(pr_outliers_path)

            if not os.path.exists(wr_outliers_path):
                wr_outliers = pd.DataFrame(
                    columns=columns)
                wr_outliers.to_csv(wr_outliers_path, index=False)
            else:
                wr_outliers = pd.read_csv(wr_outliers_path)
            if not os.path.exists(sl_outliers_path):
                sl_outliers = pd.DataFrame(
                    columns=columns)
                sl_outliers.to_csv(sl_outliers_path, index=False)
            else:
                sl_outliers = pd.read_csv(sl_outliers_path)

            pr_outliers['which_strange'] = 'delay'
            wr_outliers['which_strange'] = 'wire'
            sl_outliers['which_strange'] = 'slack'
            combined_outliers = pd.concat([pr_outliers, wr_outliers])
            combined_outliers = pd.concat([combined_outliers, sl_outliers])

            if 'id' in combined_outliers.columns:
                combined_outliers.drop('id', axis=1, inplace=True)
            combined_outliers_grouped = combined_outliers.groupby(
                list(combined_outliers.columns.drop('which_strange'))).agg({
                'which_strange': lambda
                    x: 'all' if 'delay' in x.values and 'wire' in x.values and 'slack' in x.values else (
                    x.iloc[0] if len(x) > 0 else 'unknown')
            }).reset_index()

            if mr_type == 'test_consistLayout':
                combined_outliers_grouped = combined_outliers_grouped[
                    ~(combined_outliers_grouped['route_prefer_ratio'] * combined_outliers_grouped[
                        'route_wire_ratio'] < 0)
                ]
            elif mr_type == 'test_addRouteConsist':
                combined_outliers_grouped = combined_outliers_grouped[
                    (combined_outliers_grouped['route_prefer_ratio'] > 0) & (
                                combined_outliers_grouped['route_wire_ratio'] > 0)
                    & (combined_outliers_grouped['route_slack_delta'] < 0)
                    ]

            final_path = os.path.join(output_dir, f"final_outliers_route_{mr_type}.csv")
            combined_outliers_grouped.to_csv(final_path, index=False)
            print(f"Final combined outliers saved to {final_path}")


def target_analysis(filepath, output_dir, judge_bound, target, columns, if_high_leverage=False):
    output_dir = os.path.abspath(output_dir)
    generated_files = categorize_and_save_data(filepath, output_dir)
    all_outliers = []
    columns_expected = columns

    for filename in generated_files:
        mr_value = os.path.basename(filename)
        mr_type = mr_value.replace('data_', '').replace('.csv', '')
        processed_filepath = os.path.join(output_dir, f"data_{mr_type}.csv")

        if mr_type == "test_consistLayout":
            process_consist_layout_data(filename, processed_filepath, target)
        elif mr_type == "test_addRouteConsist":
            process_add_route_data(filename, processed_filepath, target)

        boxplot_output_path = os.path.join(output_dir, f"boxplot_outliers_{mr_type}_{target}.csv")
        boxplot_image_path = os.path.join(output_dir, f"boxplot_{mr_type}_{target}.png")
        analyze_and_plot_box(processed_filepath, boxplot_output_path, boxplot_image_path, target)
        boxplot_outliers = pd.read_csv(boxplot_output_path) if os.path.exists(boxplot_output_path) else pd.DataFrame(
            columns=columns_expected)
        if not boxplot_outliers.empty:
            all_outliers.append(boxplot_outliers)

        outlier_output_path = os.path.join(output_dir, f"outliers_{mr_type}_{target}.csv")
        analyze_data_and_export_outliers(processed_filepath, outlier_output_path, judge_bound, target, mr_type,
                                         if_high_leverage)
        model_outliers = pd.read_csv(outlier_output_path) if os.path.exists(outlier_output_path) else pd.DataFrame(
            columns=columns_expected)
        if not model_outliers.empty:
            all_outliers.append(model_outliers)

        if all_outliers:
            all_outliers_df = pd.concat(all_outliers)
            duplicated_outliers = all_outliers_df[all_outliers_df.duplicated(keep='first')]
        else:
            all_outliers_df = pd.DataFrame(columns=columns_expected)
            duplicated_outliers = pd.DataFrame(columns=columns_expected)

        duplicates_path = os.path.join(output_dir, f"duplicated_{mr_type}_{target}_outliers.csv")
        duplicated_outliers.to_csv(duplicates_path, index=False)
        print(f"Duplicated outliers saved to {duplicates_path} (may be empty).")
        all_outliers = []
    return 1


def categorize_and_save_data(filepath, output_dir):
    df = pd.read_csv(filepath)
    if 'MR' not in df.columns:
        raise ValueError("The input CSV does not have an 'MR' column.")
    grouped = df.groupby('MR')
    generated_files = []
    for name, group in grouped:
        output_path = os.path.join(output_dir, f"data_{name}.csv")
        group.to_csv(output_path, index=False)
        print(f"Data for MR = {name} saved to {output_path}")
        generated_files.append(output_path)
    return generated_files


def process_consist_layout_data(filepath, output_filepath, target):
    df = pd.read_csv(filepath)

    recent_delay_type = 'prefer_ratio'
    if (target == 'route_prefer_ratio') | (target == 'route_wire_ratio') | (target == 'route_slack_delta'):
        recent_delay_type = 'route_prefer_ratio'

    def filter_rows(group):
        return group.iloc[(group[recent_delay_type].abs().argmax()): (group[recent_delay_type].abs().argmax() + 1)]

    result = df.groupby('main_blif').apply(filter_rows).reset_index(drop=True)
    result.to_csv(output_filepath, index=False)
    return result


def process_add_route_data(filepath, output_filepath, target):
    recent_delay_type = 'prefer_ratio'

    def retain_larger_abs_prefer_ratio(group):
        max_idx = group['prefer_ratio'].abs().idxmax()
        return group.loc[[max_idx]]

    def save_effective_group(group):
        positive_values = group[group[recent_delay_type] > 0]
        if not positive_values.empty:
            max_idx = positive_values[recent_delay_type].idxmax()
            return group.loc[[max_idx]]
        elif (group[recent_delay_type] < 0).any():
            return pd.DataFrame(columns=group.columns)  # Return empty DataFrame
        else:
            return group.head(1)  # Retain the first occurrence if all values are 0

    df = pd.read_csv(filepath)

    if target in ['route_prefer_ratio', 'route_wire_ratio', 'route_slack_delta']:
        recent_delay_type = 'route_prefer_ratio'
        result = df.groupby('main_blif', as_index=False).apply(save_effective_group).reset_index(drop=True)
    else:
        df = df[df[recent_delay_type] >= 0]
        result = df.groupby('main_blif', as_index=False).apply(retain_larger_abs_prefer_ratio).reset_index(drop=True)

    result.to_csv(output_filepath, index=False)
    return result


def remove_high_leverage_points_after_scaling(filepath, target):
    df = pd.read_csv(filepath)
    features = ['io_nums', 'blocks', 'total_nets', 'fanout_ave', 'fanout_max', 'logic_level']
    X = df[features]
    y = df[target]
    if X.empty or y.empty or df.shape[0] < 10:
        return df
    scaler = MinMaxScaler()
    X_scaled = scaler.fit_transform(X)
    X_scaled = sm.add_constant(X_scaled)
    model = sm.OLS(y, X_scaled).fit()
    leverage = model.get_influence().hat_matrix_diag
    p = X_scaled.shape[1] - 1
    n = X_scaled.shape[0]
    high_leverage_threshold = 2 * (p + 1) / n
    high_leverage_points = leverage > high_leverage_threshold
    df_no_leverage = df[~high_leverage_points]
    return df_no_leverage


def analyze_data_and_export_outliers(filepath, output_filepath, judge_bound, target, mr_type, if_high_leverage=False):
    if not os.path.exists(filepath):
        print(f"File not found, skipping analysis: {filepath}")
        return

    if if_high_leverage:
        df = remove_high_leverage_points_after_scaling(filepath, target)
    else:
        df = pd.read_csv(filepath)

    df.reset_index(drop=True, inplace=True)

    features = ['io_nums', 'blocks', 'total_nets', 'fanout_ave', 'fanout_max', 'logic_level']

    X = df[features]
    y = df[[target]]

    if X.empty or y.empty or df.shape[0] < 10:
        if df.shape[0] < 10:
            print(f"Samples too less: {df.shape[0]}")
        empty_columns_df = pd.DataFrame(columns=df.columns)
        empty_columns_df.to_csv(output_filepath, index=False)
        print(f"No module outliers data, {output_filepath} is empty.")
        return

    scaler_X = MinMaxScaler()
    scaler_y = MinMaxScaler()

    X_scaled = scaler_X.fit_transform(X)
    y_scaled = scaler_y.fit_transform(y)

    X_scaled = pd.DataFrame(X_scaled, columns=features)
    y_scaled = pd.DataFrame(y_scaled, columns=[target])

    X_train, X_test, y_train, y_test = train_test_split(X_scaled, y_scaled, test_size=0.2, random_state=42)

    model = MLPRegressor(hidden_layer_sizes=(100,), activation='relu', solver='adam', max_iter=500, random_state=42)
    model.fit(X_train, y_train.values.ravel())

    y_pred = model.predict(X_test)
    y_pred_df = pd.DataFrame(y_pred, columns=[target], index=y_test.index)
    residuals = y_test - y_pred_df
    residuals_std = residuals / residuals.std()
    outliers = residuals_std.abs() > judge_bound
    outlier_indices = outliers[outliers.any(axis=1)].index

    df_outliers = df.loc[df.index.isin(outlier_indices)]
    df_outliers.to_csv(output_filepath, index=False)

    return outlier_indices


# line box plot
def analyze_and_plot_box(filepath, output_csv_path, output_image_path, target):
    df = pd.read_csv(filepath)

    if df[target].isnull().all():
        empty_columns_df = pd.DataFrame(columns=df.columns)
        empty_columns_df.to_csv(output_csv_path, index=False)
        print(f"No plot data, {output_csv_path} is empty and no {output_image_path} is generated.")
        return

    plt.figure(figsize=(10, 6))
    df[target].plot(kind='box')
    plt.savefig(output_image_path)
    plt.close()

    Q1 = df[target].quantile(0.25)
    Q3 = df[target].quantile(0.75)
    IQR = Q3 - Q1
    lower_bound = Q1 - 1.5 * IQR
    upper_bound = Q3 + 1.5 * IQR


    outliers = df[(df[target] < lower_bound) | (df[target] > upper_bound)]
    outliers.to_csv(output_csv_path, index=False)
    print(f"Outliers saved to {output_csv_path} and boxplot saved to {output_image_path}")


if __name__ == '__main__':
    eda_data_analysis(
        'features.csv',
        'Debug20240902',
        3, False, False)
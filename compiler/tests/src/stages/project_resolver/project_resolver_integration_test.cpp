#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "project_resolver_test_utils.h"
#include "errors/valuascript_exception.h"
#include "stages/project_resolver/project_resolver_stage.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test {
    class ProjectResolverTest : public ::testing::Test {
    protected:
        std::string temp_dir = "test_project_workspace";
        ProjectResolverStage resolver;

        void SetUp() override {
            if (std::filesystem::exists(temp_dir)) {
                std::filesystem::remove_all(temp_dir);
            }
            std::filesystem::create_directory(temp_dir);
        }

        void TearDown() override {
            if (std::filesystem::exists(temp_dir)) {
                std::filesystem::remove_all(temp_dir);
            }
        }

        std::string create_file(const std::string &filename, const std::string &content) {
            std::filesystem::path full_path = std::filesystem::path(temp_dir) / filename;
            std::filesystem::create_directories(full_path.parent_path());

            std::ofstream out(full_path);
            out << content;
            out.close();

            return std::filesystem::weakly_canonical(full_path).string();
        }
    };

    const std::string main_file = R"(
// main.vs
// -- Alphabet (Google) Valuation August 2025 --

import "modules/wacc.vs"
import "modules/segment.vs"
import "modules/fcff.vs"

#iterations = 10_000_000

// -- R&D Capitalization --
let value_of_research_assets, current_year_amortization = get_rd()

// -- WACC --
let wacc = get_wacc()

struct Segment {
    market_share: Decimal,
    target_market_share: Decimal,
    market_size: Decimal,
    cagr: Decimal,
    cagr_variation_per_period: Decimal,
    current_margin: Decimal,
    target_margin: Decimal,
    sales_to_capital: Decimal,
    target_sales_to_capital: Decimal
}

// ==========================================
// 1. SEGMENT ASSUMPTIONS
// ==========================================

@scenario(type: "base")
let gcp_segment: Segment = {
    market_share: 11%,
    target_market_share: Uniform(min: 10%, max: 15%),
    market_size: 13_624 / 11% * 4,
    cagr: Pert(min: 15%, likely: 20%, max: 25%),
    cagr_variation_per_period: 0%,
    current_margin: 15%,
    target_margin: Uniform(min: 30%, max: 40%),
    sales_to_capital: 22%,
    target_sales_to_capital: 100%
}

@scenario(type: "base")
let yt_segment: Segment = {
    market_share: 100%,
    target_market_share: 100%,
    market_size: 9_796 * 4,
    cagr: 11%,
    cagr_variation_per_period: 0%,
    current_margin: 40%,
    target_margin: Uniform(min: 40%, max: 45%),
    sales_to_capital: 100%,
    target_sales_to_capital: 200%
}

@scenario(type: "base")
let google_network_segment: Segment = {
    market_share: 100%,
    target_market_share: 100%,
    market_size: 7_354 * 4,
    cagr: -10%,
    cagr_variation_per_period: 0%,
    current_margin: 40%,
    target_margin: 40%,
    sales_to_capital: 200%,
    target_sales_to_capital: 200%
}

@scenario(type: "base")
let google_subscriptions_segment: Segment = {
    market_share: 100%,
    target_market_share: 100%,
    market_size: 11_203 * 4,
    cagr: 10%,
    cagr_variation_per_period: 0%,
    current_margin: 20%,
    target_margin: Uniform(min: 20%, max: 25%),
    sales_to_capital: 200%,
    target_sales_to_capital: 200%
}

@scenario(type: "base")
let google_search_segment: Segment = {
    market_share: 100%,
    target_market_share: 100%,
    market_size: 54_190 * 4,
    cagr: 10%,
    cagr_variation_per_period: -15%,
    current_margin: 30%,
    target_margin: Uniform(min: 30%, max: 40%),
    sales_to_capital: 200%,
    target_sales_to_capital: 200%
}

// ==========================================
// 2. REVENUE & EBIT COMPUTATION
// ==========================================

let gcp_revenues, gcp_ebit = get_segment_data(segment: gcp_segment)
let yt_revenues, yt_ebit = get_segment_data(segment: yt_segment)
let google_network_revenues, google_network_ebit = get_segment_data(segment: google_network_segment)
let google_subscriptions_revenues, google_subscriptions_ebit = get_segment_data(segment: google_subscriptions_segment)
let google_search_revenues, google_search_ebit = get_segment_data(segment: google_search_segment)

let total_revenues = gcp_revenues + yt_revenues + google_network_revenues + google_subscriptions_revenues + google_search_revenues
let total_ebit = gcp_ebit + yt_ebit + google_network_ebit + google_subscriptions_ebit + google_search_ebit

// ==========================================
// 3. NOPAT & REINVESTMENT
// ==========================================

let future_tax_rate = get_tax_rates_progression()
let ebit_after_tax = total_ebit * (1 - future_tax_rate)

let gcp_reinvestment, gcp_ebit_dis = get_segment_reinvestment(stc: gcp_segment.sales_to_capital, target_stc: gcp_segment.target_sales_to_capital, rev: gcp_revenues, ebit: gcp_ebit, total_rev: total_revenues)
let yt_reinvestment, yt_ebit_dis = get_segment_reinvestment(stc: yt_segment.sales_to_capital, target_stc: yt_segment.target_sales_to_capital, rev: yt_revenues, ebit: yt_ebit, total_rev: total_revenues)
let google_network_reinvestment, google_network_ebit_dis = get_segment_reinvestment(stc: google_network_segment.sales_to_capital, target_stc: google_network_segment.target_sales_to_capital, rev: google_network_revenues, ebit: google_network_ebit, total_rev: total_revenues)
let google_subscriptions_reinvestment, google_subscriptions_ebit_dis = get_segment_reinvestment(stc: google_subscriptions_segment.sales_to_capital, target_stc: google_subscriptions_segment.target_sales_to_capital, rev: google_subscriptions_revenues, ebit: google_subscriptions_ebit, total_rev: total_revenues)
let google_search_reinvestment, google_search_ebit_dis = get_segment_reinvestment(stc: google_search_segment.sales_to_capital, target_stc: google_search_segment.target_sales_to_capital, rev: google_search_revenues, ebit: google_search_ebit, total_rev: total_revenues)

let total_reinvestment = gcp_reinvestment + yt_reinvestment + google_network_reinvestment + google_subscriptions_reinvestment + google_search_reinvestment
let total_current_capital = get_book_value_of_equity() + get_book_value_of_debt() - get_cash_and_marketable_securities() + value_of_research_assets
let capital_year_9 = total_current_capital + sum_series(data: total_reinvestment)

let year_10_growth = total_revenues[-1] / total_revenues[-2] - 1
let year_10_roi = total_ebit[-1] / capital_year_9
let reinvestment_year_10 = year_10_growth / year_10_roi * ebit_after_tax[-1]

// ==========================================
// 4. FCFF & VALUATION
// ==========================================

let sum_of_d_fcff = get_sum_of_fcff(ebit: ebit_after_tax, reinvestment: reinvestment_year_10, rate: wacc, total_reinv: total_reinvestment)

let total_ebit_dis = gcp_ebit_dis + yt_ebit_dis + google_network_ebit_dis + google_subscriptions_ebit_dis + google_search_ebit_dis
let d_tv = get_discounted_terminal_value(rev: total_revenues, ebit: total_ebit_dis, tax: future_tax_rate, rate: wacc)

let value_of_common_equity = sum_of_d_fcff + d_tv
let final_value_of_common_equity = value_of_common_equity - get_book_value_of_debt() + get_cash_and_marketable_securities()

let value_per_share = final_value_of_common_equity / get_shares_outstanding()

#output = value_per_share
#output_file = "results.csv"
)";

    const std::string wacc_file = R"(
// wacc.vs
import "../assumptions/assumptions.vs"

@export func get_wacc() -> scalar {
    let k_equity = rf + (beta * erp)
    let k_debt = (rf + bond_spread) * (1 - marginal_tax_rate)
    let total_k = equity_value + book_value_of_debt
    let equity_percentage = equity_value / total_k
    let debt_percentage = 1 - equity_percentage

    let wacc = k_debt * debt_percentage + k_equity * equity_percentage

    return wacc
}
)";

    const std::string segment_file = R"(
// segment.vs
import "../assumptions/assumptions.vs"

@export func get_segment_data(segment: Segment) -> vector, vector {
    """Computes the segment data: revenues and operating income for a segment based on its overall market of reference (TAM)    """

    let market_total_revenues = grow_series(start: segment.market_size, rate: segment.cagr, periods: periods)
    let interim_market_share = interpolate_series(start: segment.market_share, target: segment.target_market_share, periods: periods)
    let revenues = market_total_revenues * interim_market_share
    let margins_interim = interpolate_series(start: segment.current_margin, target: segment.target_margin, periods: periods)
    let operating_income = revenues * margins_interim

    return revenues, operating_income
}

@export func get_base_segment_data(segment: Segment) -> vector, vector  {
    """
    Computes the segment data: revenues and operating income for a segment where the segment represents the majority on its overall market of reference (TAM).

    In other words when it makes little sense to try to model the overall market because the segment is niche or because it is a very well known and established product (e.g. YouTube)
    """

    let revenues = grow_series(start: segment.market_size, rate: segment.cagr, periods: periods)
    let margins_interim = interpolate_series(start: segment.current_margin, target: segment.target_margin, periods: periods)
    let operating_margin = revenues * margins_interim

    return revenues, operating_margin
}

@export func get_base_segment_data_from_cagr_vector(segment: Segment) -> vector, vector  {
    """
    Computes the segment data: revenues and operating income for a segment where the segment represents the majority on its overall market of reference (TAM).

    In other words when it makes little sense to try to model the overall market because the segment is niche or because it is a very well known and established product (e.g. YouTube).

    It calculates the revenues using the vector of rates for each compounding period instead of a single cagr value.
    """

    let cagr_interim = grow_series(start: segment.cagr, rate: segment.cagr_variation_per_period, periods: periods)
    let revenues = compound_series(start: segment.market_size, rates: cagr_interim)
    let margins_interim = interpolate_series(start: segment.current_margin, target: segment.target_margin, periods: periods)
    let operating_margin = revenues * margins_interim

    return revenues, operating_margin
}

@export func get_segment_reinvestment(segment: Segment,
                                      revenues: vector,
                                      ebit: vector,
                                      total_revenues: vector) -> (vector, vector) {

    let sales_to_capital = interpolate_series(start: segment.sales_to_capital, target: segment.target_sales_to_capital, periods: get_periods())
    let revenues_weight_percentage = revenues / total_revenues
    let ebit_margin_weight_percentage = ebit / revenues
    let ebit_dis = ebit_margin_weight_percentage * revenues_weight_percentage
    let reinvestment = series_delta(data: revenues) / sales_to_capital[:-1]

    return (reinvestment, ebit_dis)
}
)";

    const std::string fcff_file = R"(
// fcff.vs
import "../assumptions/assumptions.vs"
import "wacc.vs"

@export func get_sum_of_fcff(ebit_after_tax: vector,
                     reinvestment_last_year: scalar,
                     wacc: scalar,
                     total_reinvestment: vector) -> scalar {

    let d_fcff_last = (ebit_after_tax[-1] - reinvestment_last_year) / (1 + wacc)^periods

    let d_fcff_to_n_minus_1 = npv(rate: wacc, cash_flows: (ebit_after_tax[:-1] - total_reinvestment))

    let sum_of_d_fcff = d_fcff_to_n_minus_1 + d_fcff_last

    return sum_of_d_fcff
}

@export func get_discounted_terminal_value(total_revenues: vector,
                                   total_ebit_dis: vector,
                                   future_tax_rate: vector,
                                   wacc: scalar) -> scalar {

    let final_revenues = total_revenues[-1] * (1 + rf)
    let final_nopat = final_revenues * total_ebit_dis[-1] * (1 - future_tax_rate[-1])
    let final_reinvestment = rf / return_on_capital_in_perpetuity * final_nopat

    let terminal_fcff = final_nopat - final_reinvestment
    let tv = terminal_fcff / (wacc - rf)
    let d_tv = tv / (1 + wacc)^periods
    return d_tv
}
)";

    const std::string assumptions_file = R"(
// assumptions.vs

@export let periods = 10
@export let rf = 4.5%
@export let erp = 5%
@export let beta = 1.05
@export let bond_spread = 0.74%
@export let marginal_tax_rate = 21%
@export let effective_tax_rate = 17%
@export let share_price = 220
@export let shares_outstanding = 11_000_000
@export let book_value_of_equity = 300_000
@export let book_value_of_debt = 50_000
@export let cash_and_marketable_securities = 120_000
@export let equity_value = shares_outstanding * share_price
)";


    TEST_F(ProjectResolverTest, ResolvesAlphabetValuationIntegrationProject) {
        // 1. Create assumptions.vs (The Leaf Node)
        std::string assumptions_path = create_file("assumptions/assumptions.vs", assumptions_file);

        // 2. Create wacc.vs (Depends on assumptions)
        std::string wacc_path = create_file("modules/wacc.vs", wacc_file);

        // 3. Create segment.vs (Depends on assumptions)
        std::string segment_path = create_file("modules/segment.vs", segment_file);

        // 4. Create fcff.vs (Depends on assumptions AND wacc)
        std::string fcff_path = create_file("modules/fcff.vs", fcff_file);

        // 5. Create main.vs (The Root Node - Depends on wacc, segment, and fcff)
        // We include the exact scenario syntax and segment struct initialization you provided.
        std::string main_path = create_file("main.vs", main_file);

        // -- RUN THE RESOLVER --
        auto project = test::run_resolver(main_path);

        // -- VERIFY THE GRAPH GEOMETRY --

        // 1. All 5 files must be discovered and parsed exactly once [cite: 1]
        EXPECT_EQ(project.modules.size(), 5);

        // 2. Verify the strict Topological Order (Depth-First Search)
        ASSERT_EQ(project.topological_order.size(), 5);

        // assumptions.vs must be first. It is the absolute bottom of the graph, imported by everyone. [cite: 9, 12, 21]
        EXPECT_EQ(project.topological_order[0], assumptions_path);

        // wacc.vs is imported first by main.vs, so its DFS branch completes next.
        EXPECT_EQ(project.topological_order[1], wacc_path);

        // segment.vs is imported second by main.vs. Its only dependency (assumptions) is already resolved. [cite: 2, 12]
        EXPECT_EQ(project.topological_order[2], segment_path);

        // fcff.vs is imported third. Both its dependencies (assumptions and wacc) are already resolved, so it completes immediately. [cite: 2, 9]
        EXPECT_EQ(project.topological_order[3], fcff_path);

        // main.vs must be absolute last, acting as the capstone of the tree.
        EXPECT_EQ(project.topological_order[4], main_path);
    }
}

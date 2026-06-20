#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "project_resolver_test_utils.h"
#include "midend/project_resolver/project_resolver_stage.h"
#include "utils/pid.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class ProjectResolverTest : public ::testing::Test
    {
    protected:
        std::filesystem::path temp_dir;

        void SetUp() override
        {
            temp_dir = generate_test_workspace("vs_test", reinterpret_cast<uintptr_t>(this));
        }

        void TearDown() override
        {
            cleanup_test_workspace(temp_dir);
        }

        std::string create_file(const std::string& filename, const std::string& content)
        {
            std::filesystem::path full_path = temp_dir / filename;
            std::filesystem::create_directories(full_path.parent_path());

            std::ofstream out(full_path);
            out << content;
            out.close();

            return std::filesystem::weakly_canonical(full_path).string();
        }
    };

    const std::string main_file = R"(
import "assumptions/assumptions.vs"
import "modules/segment.vs"
import "modules/fcff.vs"
import "modules/company.vs"
import "modules/valuation.vs"

#iterations = 10_000_000

// ==========================================
// 1. COMPANY ASSUMPTIONS
// ==========================================

@scenario(type: "base")
let company_assumptions: CompanyAssumptions = {
    periods: 10,
    @correlated(with: [ { name: self.erp, direction: CorrelationDirection.Negative } ])
    rf: 4.5%,
    @correlated(with: [ { name: self.rf, direction: CorrelationDirection.Negative } ])
    erp: 5%,
    beta: 1.05,
    bond_spread: 0.74%,
    marginal_tax_rate: 21%,
    effective_tax_rate: 17%,
    share_price: 220,
    shares_outstanding: 11_000_000,
    book_value_of_equity: 300_000,
    book_value_of_debt: 50_000,
    cash_and_marketable_securities: 120_000,
    equity_value: 11_000_000 * 220
}

// ==========================================
// 2. SEGMENT ASSUMPTIONS
// ==========================================

@scenario(type: "base") 
let gcp_segment: Segment = {
    market_share: 11%,
    target_market_share: Uniform(min: 10%, max: 15%),
    market_size: 13_624 / self.market_share * 4,
    cagr: Pert(min: 15%, likely: 20%, max: 25%),
    cagr_variation_per_period: 0%,
    current_margin: 15%,
    @correlated(with: [ { name: gcp_revenues, direction: CorrelationDirection.Positive } ])
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
    @correlated(with: [ { name: yt_revenues, direction: CorrelationDirection.Positive } ])
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
    @correlated(with: [ { name: google_subscriptions_revenues, direction: CorrelationDirection.Positive } ])
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
    @correlated(with: [ { name: google_search_revenues, direction: CorrelationDirection.Positive } ])
    target_margin: Uniform(min: 30%, max: 40%),
    sales_to_capital: 200%,
    target_sales_to_capital: 200%
}

// ==========================================
// 2. ORCHESTRATION & VALUATION
// ==========================================

let alphabet: Company = {
    assumptions: company_assumptions,
    gcp_segment: gcp_segment,
    yt_segment: yt_segment,
    google_network_segment: google_network_segment,
    google_subscriptions_segment: google_subscriptions_segment,
    google_search_segment: google_search_segment
}

let model: ValuationModel = {
    company: alphabet
}

let value_per_share = model.compute_value_per_share()

#output = value_per_share
#output_file = "results.csv"
)";

    const std::string assumptions_file = R"(
@export struct CompanyAssumptions {
    periods: int,
    rf: scalar,
    erp: scalar,
    beta: scalar,
    bond_spread: scalar,
    marginal_tax_rate: scalar,
    effective_tax_rate: scalar,
    share_price: scalar,
    shares_outstanding: scalar,
    book_value_of_equity: scalar,
    book_value_of_debt: scalar,
    cash_and_marketable_securities: scalar,
    equity_value: scalar
}

extension CompanyAssumptions {
    func get_wacc() -> scalar {
        let k_equity = self.rf + (self.beta * self.erp)
        let k_debt = (self.rf + self.bond_spread) * (1 - self.marginal_tax_rate)
        let total_k = self.equity_value + self.book_value_of_debt
        let equity_percentage = self.equity_value / total_k
        let debt_percentage = 1 - equity_percentage

        let wacc = k_debt * debt_percentage + k_equity * equity_percentage

        return wacc
    }

    func get_tax_rates_progression() -> vector {
        return interpolate_series(start: self.effective_tax_rate, target: self.marginal_tax_rate, periods: self.periods)
    }
}
)";

    const std::string company_file = R"(
import "../assumptions/assumptions.vs"
import "segment.vs"

@export struct CompanyFinancials {
    total_revenues: vector,
    total_ebit: vector,
    ebit_after_tax: vector,
    total_reinvestment: vector,
    total_ebit_dis: vector,
    future_tax_rate: vector
}

@export struct Company {
    assumptions: CompanyAssumptions,
    gcp_segment: Segment,
    yt_segment: Segment,
    google_network_segment: Segment,
    google_subscriptions_segment: Segment,
    google_search_segment: Segment
}

extension Company {
    func compute_financials() -> CompanyFinancials {
        let periods = self.assumptions.periods
        let gcp_revenues, gcp_ebit = self.gcp_segment.get_data(periods: periods)
        let yt_revenues, yt_ebit = self.yt_segment.get_data(periods: periods)
        let google_network_revenues, google_network_ebit = self.google_network_segment.get_data(periods: periods)
        let google_subscriptions_revenues, google_subscriptions_ebit = self.google_subscriptions_segment.get_data(periods: periods)
        let google_search_revenues, google_search_ebit = self.google_search_segment.get_data(periods: periods)

        let total_revenues = gcp_revenues + yt_revenues + google_network_revenues + google_subscriptions_revenues + google_search_revenues
        let total_ebit = gcp_ebit + yt_ebit + google_network_ebit + google_subscriptions_ebit + google_search_ebit

        let future_tax_rate = self.assumptions.get_tax_rates_progression()
        let ebit_after_tax = total_ebit * (1 - future_tax_rate)

        let gcp_reinvestment, gcp_ebit_dis = self.gcp_segment.get_reinvestment(periods: periods, revenues: gcp_revenues, ebit: gcp_ebit, total_revenues: total_revenues)
        let yt_reinvestment, yt_ebit_dis = self.yt_segment.get_reinvestment(periods: periods, revenues: yt_revenues, ebit: yt_ebit, total_revenues: total_revenues)
        let google_network_reinvestment, google_network_ebit_dis = self.google_network_segment.get_reinvestment(periods: periods, revenues: google_network_revenues, ebit: google_network_ebit, total_revenues: total_revenues)
        let google_subscriptions_reinvestment, google_subscriptions_ebit_dis = self.google_subscriptions_segment.get_reinvestment(periods: periods, revenues: google_subscriptions_revenues, ebit: google_subscriptions_ebit, total_revenues: total_revenues)
        let google_search_reinvestment, google_search_ebit_dis = self.google_search_segment.get_reinvestment(periods: periods, revenues: google_search_revenues, ebit: google_search_ebit, total_revenues: total_revenues)

        let total_reinvestment = gcp_reinvestment + yt_reinvestment + google_network_reinvestment + google_subscriptions_reinvestment + google_search_reinvestment
        
        let total_ebit_dis = gcp_ebit_dis + yt_ebit_dis + google_network_ebit_dis + google_subscriptions_ebit_dis + google_search_ebit_dis

        let financials: CompanyFinancials = {
            total_revenues: total_revenues,
            total_ebit: total_ebit,
            ebit_after_tax: ebit_after_tax,
            total_reinvestment: total_reinvestment,
            total_ebit_dis: total_ebit_dis,
            future_tax_rate: future_tax_rate
        }

        return financials
    }
}
)";

    const std::string fcff_file = R"(
import "../assumptions/assumptions.vs"

@export struct DiscountedCashFlow {
    ebit_after_tax: vector,
    total_reinvestment: vector,
    assumptions: CompanyAssumptions
}

extension DiscountedCashFlow {

    func get_sum_of_fcff(reinvestment_last_year: scalar) -> scalar {
        let wacc = self.assumptions.get_wacc()
        let periods = self.assumptions.periods
        let d_fcff_last = (self.ebit_after_tax[-1] - reinvestment_last_year) / (1 + wacc)^periods

        let d_fcff_to_n_minus_1 = npv(rate: wacc, cash_flows: (self.ebit_after_tax[:-1] - self.total_reinvestment))

        let sum_of_d_fcff = d_fcff_to_n_minus_1 + d_fcff_last

        return sum_of_d_fcff
    }

    func get_discounted_terminal_value(total_revenues: vector,
                                       total_ebit_dis: vector,
                                       future_tax_rate: vector) -> scalar {

        let rf = self.assumptions.rf
        let wacc = self.assumptions.get_wacc()
        let periods = self.assumptions.periods

        let final_revenues = total_revenues[-1] * (1 + rf)
        let final_nopat = final_revenues * total_ebit_dis[-1] * (1 - future_tax_rate[-1])
        let final_reinvestment = rf / return_on_capital_in_perpetuity * final_nopat

        let terminal_fcff = final_nopat - final_reinvestment
        let tv = terminal_fcff / (wacc - rf)
        let d_tv = tv / (1 + wacc)^periods
        
        return d_tv
    }
}
)";

    const std::string segment_file = R"(
@export struct Segment {
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

extension Segment {

    func get_data(periods: int) -> vector, vector {
        """Computes the segment data: revenues and operating income for a segment based on its overall market of reference TAM"""

        let market_total_revenues = grow_series(start: self.market_size, rate: self.cagr, periods: periods)
        let interim_market_share = interpolate_series(start: self.market_share, target: self.target_market_share, periods: periods)
        let revenues = market_total_revenues * interim_market_share
        let margins_interim = interpolate_series(start: self.current_margin, target: self.target_margin, periods: periods)
        let operating_income = revenues * margins_interim   

        return revenues, operating_income
    }

    func get_base_data(periods: int) -> vector, vector  {
        """
        Computes the segment data: revenues and operating income for a segment where the segment represents the majority on its overall market of reference TAM.

        In other words when it makes little sense to try to model the overall market because the segment is niche or because it is a very well known and established product (e.g. YouTube)
        """

        let revenues = grow_series(start: self.market_size, rate: self.cagr, periods: periods)
        let margins_interim = interpolate_series(start: self.current_margin, target: self.target_margin, periods: periods)
        let operating_margin = revenues * margins_interim

        return revenues, operating_margin
    }

    func get_base_data_from_cagr_vector(periods: int) -> vector, vector  {
        """
        Computes the segment data: revenues and operating income for a segment where the segment represents the majority on its overall market of reference TAM.

        In other words when it makes little sense to try to model the overall market because the segment is niche or because it is a very well known and established product (e.g. YouTube).

        It calculates the revenues using the vector of rates for each compounding period instead of a single cagr value.
        """

        let cagr_interim = grow_series(start: self.cagr, rate: self.cagr_variation_per_period, periods: periods)
        let revenues = compound_series(start: self.market_size, rates: cagr_interim)
        let margins_interim = interpolate_series(start: self.current_margin, target: self.target_margin, periods: periods)
        let operating_margin = revenues * margins_interim

        return revenues, operating_margin
    }

    func get_reinvestment(periods: int, revenues: vector, ebit: vector, total_revenues: vector) -> (vector, vector) {
        let sales_to_capital = interpolate_series(start: self.sales_to_capital, target: self.target_sales_to_capital, periods: periods)
        let revenues_weight_percentage = revenues / total_revenues
        let ebit_margin_weight_percentage = ebit / revenues
        let ebit_dis = ebit_margin_weight_percentage * revenues_weight_percentage
        let reinvestment = series_delta(data: revenues) / sales_to_capital[:-1]

        return (reinvestment, ebit_dis)
    }
}
)";

    const std::string valuation_file = R"(
import "../assumptions/assumptions.vs"
import "company.vs"
import "fcff.vs"

@export 

@export struct ValuationModel {
    company: Company
}

extension ValuationModel {

    @private func get_rd() -> (scalar, scalar, scalar) {
        let amortization_period = 3
        let current_expense = 27_364 + 49_326 - 23_763
        let past_expenses = [49_326, 45_427, 39_500]

        let capitalized_assets, amortization_current_year = capitalize_expense(current_expense: current_expense, past_expenses: past_expenses, amortization_period: amortization_period)

        return (capitalized_assets, amortization_current_year, current_expense)
    }

    func compute_value_per_share() -> scalar {
        let value_of_research_assets, current_year_amortization, current_expense = self.get_rd()
        let financials = self.company.compute_financials()

        let adjusted_total_ebit = financials.total_ebit + (current_expense - current_year_amortization)
        let adjusted_ebit_after_tax = adjusted_total_ebit * (1 - financials.future_tax_rate)

        let total_current_capital = self.company.assumptions.book_value_of_equity + self.company.assumptions.book_value_of_debt - self.company.assumptions.cash_and_marketable_securities + value_of_research_assets
        let capital_year_9 = total_current_capital + sum_series(data: financials.total_reinvestment)

        let year_10_growth = financials.total_revenues[-1] / financials.total_revenues[-2] - 1
        let year_10_roi = adjusted_total_ebit[-1] / capital_year_9
        let reinvestment_year_10 = year_10_growth / year_10_roi * adjusted_ebit_after_tax[-1]

        let dcf: DiscountedCashFlow = {
            ebit_after_tax: adjusted_ebit_after_tax,
            total_reinvestment: financials.total_reinvestment,
            assumptions: self.company.assumptions
        }

        let sum_of_d_fcff = dcf.get_sum_of_fcff(reinvestment_last_year: reinvestment_year_10)

        let d_tv = dcf.get_discounted_terminal_value(total_revenues: financials.total_revenues, total_ebit_dis: financials.total_ebit_dis, future_tax_rate: financials.future_tax_rate)

        let value_of_common_equity = sum_of_d_fcff + d_tv
        let final_value_of_common_equity = value_of_common_equity - self.company.assumptions.book_value_of_debt + self.company.assumptions.cash_and_marketable_securities

        let value_per_share = final_value_of_common_equity / self.company.assumptions.shares_outstanding

        return value_per_share
    }
}
)";

    TEST_F(ProjectResolverTest, ResolvesAlphabetValuationIntegrationProject)
    {
        std::string assumptions_path = create_file("assumptions/assumptions.vs", assumptions_file);

        std::string segment_path = create_file("modules/segment.vs", segment_file);

        std::string company_path = create_file("modules/company.vs", company_file);

        std::string fcff_path = create_file("modules/fcff.vs", fcff_file);

        std::string valuation_path = create_file("modules/valuation.vs", valuation_file);

        std::string main_path = create_file("main.vs", main_file);

        auto project = run_resolver(main_path);

        EXPECT_EQ(project.modules.size(), 6);

        ASSERT_EQ(project.topological_order.size(), 6);

        EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), assumptions_path),
                  project.topological_order.end());
        EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), segment_path),
                  project.topological_order.end());
        EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), company_path),
                  project.topological_order.end());
        EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), fcff_path),
                  project.topological_order.end());
        EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), valuation_path),
                  project.topological_order.end());
        EXPECT_NE(std::find(project.topological_order.begin(), project.topological_order.end(), main_path),
                  project.topological_order.end());

        EXPECT_EQ(project.topological_order[5], main_path);
    }
}

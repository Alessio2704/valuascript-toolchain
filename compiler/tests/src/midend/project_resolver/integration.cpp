#include <gtest/gtest.h>
#include "project_resolver_test_base.h"

namespace valuascript::compiler::test
{
    class ProjectResolverIntegrationTest : public ProjectResolverTestBase
    {
    protected:
        const std::string main_file = R"(
import "assumptions/assumptions.vs"
import "modules/segment.vs"
import "modules/fcff.vs"
import "modules/company.vs"
import "modules/valuation.vs"

#iterations = 10_000_000

@scenario(type: "base")
let company_assumptions: CompanyAssumptions = {
    periods: 10,
    rf: 4.5%,
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

let alphabet: Company = {
    assumptions: company_assumptions,
    gcp_segment: gcp_segment
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
    rf: scalar
}
)";

        const std::string company_file = R"(
import "../assumptions/assumptions.vs"
import "segment.vs"

@export struct Company {
    assumptions: CompanyAssumptions,
    gcp_segment: Segment
}
)";

        const std::string fcff_file = R"(
import "../assumptions/assumptions.vs"

@export struct DiscountedCashFlow {
    assumptions: CompanyAssumptions
}
)";

        const std::string segment_file = R"(
@export struct Segment {
    market_share: Decimal
}
)";

        const std::string valuation_file = R"(
import "../assumptions/assumptions.vs"
import "company.vs"
import "fcff.vs"

@export struct ValuationModel {
    company: Company
}
)";
    };

    TEST_F(ProjectResolverIntegrationTest, ResolvesAlphabetValuationIntegrationProject)
    {
        std::string assumptions_path = CreateFile("assumptions/assumptions.vs", assumptions_file);
        std::string segment_path = CreateFile("modules/segment.vs", segment_file);
        std::string company_path = CreateFile("modules/company.vs", company_file);
        std::string fcff_path = CreateFile("modules/fcff.vs", fcff_file);
        std::string valuation_path = CreateFile("modules/valuation.vs", valuation_file);
        std::string main_path = CreateFile("main.vs", main_file);

        ExpectResolverContainsModules(main_path, 6,
            {assumptions_path, segment_path, company_path, fcff_path, valuation_path, main_path},
            main_path
        );
    }
}

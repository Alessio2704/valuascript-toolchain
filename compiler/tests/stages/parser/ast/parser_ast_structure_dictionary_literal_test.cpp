#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"

using namespace valuascript;
using namespace valuascript::compiler;

class AstDictLiteralTest : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string& code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        auto parser_result = parser.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        });
        
        return std::any_cast<std::shared_ptr<Program>>(parser_result.data);
    }
    
    Expression* get_assigned_value(const std::shared_ptr<Program>& ast) {
        if (ast->execution_steps.empty()) return nullptr;
        auto assign = dynamic_cast<Assignment*>(ast->execution_steps[0].get());
        if (!assign) return nullptr;
        return assign->value.get();
    }
};

TEST_F(AstDictLiteralTest, ValidatesEmptyDictionary) {
    // Proves the parser correctly identifies opening and closing braces with no contents.
    
    auto ast = parse_code("let empty = {}");
    auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));
    
    ASSERT_NE(dict_val, nullptr) << "Assigned value must be a DictLiteral";
    EXPECT_EQ(dict_val->pairs.size(), 0) << "Empty dictionary must have 0 pairs";
}

TEST_F(AstDictLiteralTest, ValidatesFlatDictionary) {
    // Proves the parser accurately captures key-value pairs with primitive expressions.
    
    auto ast = parse_code("let model = { cagr: 0.05, yrs: 10 }");
    auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));
    
    ASSERT_NE(dict_val, nullptr);
    ASSERT_EQ(dict_val->pairs.size(), 2);
    
    // First Pair: cagr: 0.05
    EXPECT_EQ(dict_val->pairs[0].first, "cagr");
    auto val0 = dynamic_cast<NumberLiteral*>(dict_val->pairs[0].second.get());
    ASSERT_NE(val0, nullptr);
    EXPECT_EQ(val0->value, "0.05");
    
    // Second Pair: yrs: 10
    EXPECT_EQ(dict_val->pairs[1].first, "yrs");
    auto val1 = dynamic_cast<NumberLiteral*>(dict_val->pairs[1].second.get());
    ASSERT_NE(val1, nullptr);
    EXPECT_EQ(val1->value, "10");
}

TEST_F(AstDictLiteralTest, ValidatesNestedDictionaryWithComplexExpressions) {
    // Proves that dictionaries safely house nested dictionaries and math expressions
    // without corrupting the parser's internal loop state.
    
    auto ast = parse_code("let nested = { base: { rate: 0.05 }, stress: wacc * 1.2 }");
    auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));
    
    ASSERT_NE(dict_val, nullptr);
    ASSERT_EQ(dict_val->pairs.size(), 2);
    
    // ==========================================
    // PAIR 0: base: { rate: 0.05 }
    // ==========================================
    EXPECT_EQ(dict_val->pairs[0].first, "base");
    
    auto inner_dict = dynamic_cast<DictLiteral*>(dict_val->pairs[0].second.get());
    ASSERT_NE(inner_dict, nullptr) << "Value for 'base' must be a nested DictLiteral";
    ASSERT_EQ(inner_dict->pairs.size(), 1);
    
    EXPECT_EQ(inner_dict->pairs[0].first, "rate");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(inner_dict->pairs[0].second.get())->value, "0.05");
    
    // ==========================================
    // PAIR 1: stress: wacc * 1.2
    // ==========================================
    EXPECT_EQ(dict_val->pairs[1].first, "stress");
    
    auto math_op = dynamic_cast<BinaryExpression*>(dict_val->pairs[1].second.get());
    ASSERT_NE(math_op, nullptr) << "Value for 'stress' must be a BinaryExpression";
    EXPECT_EQ(math_op->op, TokenType::Star);
    
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(math_op->left.get())->name, "wacc");
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(math_op->right.get())->value, "1.2");
}

TEST_F(AstDictLiteralTest, ValidatesOmnibusDictionaryWithAllExpressionTypes) {
    // Proves the DictLiteral seamlessly delegates parsing to every corner of the expression grammar.

    std::string code =
        "let omnibus = {\n"
        "    scalar: -100,\n"
        "    equation: (base + 0.05) * multiplier,\n"
        "    logic: not is_valid,\n"
        "    group: (1, a * b),\n"
        "    arr: [10, 20],\n"
        "    subset: history[0 : 10],\n"
        "    invoke: calc_risk(rate: 0.08),\n"
        "    nested: { inner: not not flag }\n"
        "}";

    auto ast = parse_code(code);
    auto dict_val = dynamic_cast<DictLiteral*>(get_assigned_value(ast));

    ASSERT_NE(dict_val, nullptr) << "Assigned value must be a DictLiteral";
    ASSERT_EQ(dict_val->pairs.size(), 8) << "Omnibus dictionary must hold exactly 8 pairs";

    // ==========================================
    // PAIR 0: Unary Expression (scalar: -100)
    // ==========================================
    EXPECT_EQ(dict_val->pairs[0].first, "scalar");
    auto pair0_unary = dynamic_cast<UnaryExpression*>(dict_val->pairs[0].second.get());
    ASSERT_NE(pair0_unary, nullptr);
    EXPECT_EQ(pair0_unary->op, TokenType::Minus);
    EXPECT_EQ(dynamic_cast<NumberLiteral*>(pair0_unary->right.get())->value, "100");

    // ==========================================
    // PAIR 1: Overridden Binary Math (equation: (base + 0.05) * multiplier)
    // ==========================================
    EXPECT_EQ(dict_val->pairs[1].first, "equation");
    auto pair1_mult = dynamic_cast<BinaryExpression*>(dict_val->pairs[1].second.get());
    ASSERT_NE(pair1_mult, nullptr);
    EXPECT_EQ(pair1_mult->op, TokenType::Star);

    auto pair1_add = dynamic_cast<BinaryExpression*>(pair1_mult->left.get());
    ASSERT_NE(pair1_add, nullptr);
    EXPECT_EQ(pair1_add->op, TokenType::Plus); // Proves parentheses were stripped and precedence applied
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair1_mult->right.get())->name, "multiplier");

    // ==========================================
    // PAIR 2: Logical Unary (logic: not is_valid)
    // ==========================================
    EXPECT_EQ(dict_val->pairs[2].first, "logic");
    auto pair2_not = dynamic_cast<UnaryExpression*>(dict_val->pairs[2].second.get());
    ASSERT_NE(pair2_not, nullptr);
    // Adjust token type based on your enum for the 'not' keyword
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair2_not->right.get())->name, "is_valid");

    // ==========================================
    // PAIR 3: Tuple Literal (group: (1, a * b))
    // ==========================================
    EXPECT_EQ(dict_val->pairs[3].first, "group");
    auto pair3_tuple = dynamic_cast<TupleLiteral*>(dict_val->pairs[3].second.get());
    ASSERT_NE(pair3_tuple, nullptr);
    ASSERT_EQ(pair3_tuple->elements.size(), 2);
    EXPECT_EQ(dynamic_cast<BinaryExpression*>(pair3_tuple->elements[1].get())->op, TokenType::Star);

    // ==========================================
    // PAIR 4: Vector Literal (arr: [10, 20])
    // ==========================================
    EXPECT_EQ(dict_val->pairs[4].first, "arr");
    auto pair4_vec = dynamic_cast<TensorLiteral*>(dict_val->pairs[4].second.get());
    ASSERT_NE(pair4_vec, nullptr);
    ASSERT_EQ(pair4_vec->elements.size(), 2);

    // ==========================================
    // PAIR 5: Tensor Slicing (subset: history[0 : 10])
    // ==========================================
    EXPECT_EQ(dict_val->pairs[5].first, "subset");
    auto pair5_slice = dynamic_cast<BracketAccess*>(dict_val->pairs[5].second.get());
    ASSERT_NE(pair5_slice, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair5_slice->target.get())->name, "history");
    EXPECT_EQ(dynamic_cast<BinaryExpression*>(pair5_slice->index.get())->op, TokenType::Colon);

    // ==========================================
    // PAIR 6: Function Call (invoke: calc_risk(rate: 0.08))
    // ==========================================
    EXPECT_EQ(dict_val->pairs[6].first, "invoke");
    auto pair6_call = dynamic_cast<FunctionCall*>(dict_val->pairs[6].second.get());
    ASSERT_NE(pair6_call, nullptr);
    EXPECT_EQ(dynamic_cast<IdentifierAccess*>(pair6_call->target.get())->name, "calc_risk");
    ASSERT_EQ(pair6_call->arguments.size(), 1);

    // ==========================================
    // PAIR 7: Nested Dict with Chained Unary (nested: { inner: !!flag })
    // ==========================================
    EXPECT_EQ(dict_val->pairs[7].first, "nested");
    auto pair7_dict = dynamic_cast<DictLiteral*>(dict_val->pairs[7].second.get());
    ASSERT_NE(pair7_dict, nullptr);
    ASSERT_EQ(pair7_dict->pairs.size(), 1);

    EXPECT_EQ(pair7_dict->pairs[0].first, "inner");
    auto inner_unary_outer = dynamic_cast<UnaryExpression*>(pair7_dict->pairs[0].second.get());
    ASSERT_NE(inner_unary_outer, nullptr);
    EXPECT_EQ(inner_unary_outer->op, TokenType::Not);

    auto inner_unary_inner = dynamic_cast<UnaryExpression*>(inner_unary_outer->right.get());
    ASSERT_NE(inner_unary_inner, nullptr);
    EXPECT_EQ(inner_unary_inner->op, TokenType::Not);
}
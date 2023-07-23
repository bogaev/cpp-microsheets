#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

FormulaError::FormulaError(Category category)
    : category_(category)
{
}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return this->category_ == rhs.GetCategory();
}

std::string_view FormulaError::ToString() const { 
    switch (category_)
    {
    case FormulaError::Category::Ref:
        return "#REF!"sv;
        break;
    case FormulaError::Category::Value:
        return "#VALUE!"sv;
        break;
    case FormulaError::Category::Div0:
        return "#DIV/0!"sv;
        break;
    default:
        break;
    };
    return {};
}

namespace {

class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression);

    Value Evaluate(const SheetInterface& sheet) const override {
        Value ret;
        auto value_getter = [&sheet](const Position& pos) {
            return sheet.GetCell(pos);
        };
        try {
            ret = ast_.Execute(value_getter);
        }
        catch (const FormulaError& exc) {
            ret = exc;
        }
        return ret;
    }

    std::string GetExpression() const override {
        std::stringstream clean_formula;
        ast_.PrintFormula(clean_formula);
        return clean_formula.str();
    }

    std::vector<Position> GetReferencedCells() const override {
        return refs_;
    }

private:
    FormulaAST ast_;
    std::vector<Position> refs_;
};

Formula::Formula(std::string expression)
try : ast_(ParseFormulaAST(expression)) {
    std::vector<Position> refs;
    auto refs_list = ast_.GetCells();
    if (refs_list.empty()) {
        return;
    }
    for (auto& pos : refs_list) {
        refs.push_back(std::move(pos));
    }
    auto last = std::unique(refs.begin(), refs.end());
    refs.erase(last, refs.end());
    refs_ = refs;
}
catch (...) {
    throw FormulaException("FormulaException!");
}

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}

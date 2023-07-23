#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>

using namespace std;

class Impl {
public:
	using Value = CellInterface::Value;
	virtual ~Impl() = default;

	virtual Value GetValue() const = 0;
	virtual string GetText() const = 0;
	virtual std::vector<Position> GetReferencedCells() const = 0;

protected:
	Impl() = default;
};

namespace {

class EmptyImpl : public Impl {
public:
	EmptyImpl() = default;
	~EmptyImpl() = default;

	Value GetValue() const override {
		return ""s;
	};
	string GetText() const override {
		return ""s;
	};
	std::vector<Position> GetReferencedCells() const override {
		return {};
	};
};

class TextImpl : public Impl {
public:
	TextImpl(string str) : text_(str) {}
	~TextImpl() = default;

	Value GetValue() const override {
		if (text_[0] == ESCAPE_SIGN) {
			return text_.substr(1);
		}
		else if ( IsNumber(text_) ) {
			return static_cast<double>(std::stoi(text_));
		}
		return text_;
	}
	string GetText() const override {
		return text_;
	};
	std::vector<Position> GetReferencedCells() const override {
		return {};
	};

private:
	bool IsNumber(const std::string& s) const {
		return !s.empty() && std::find_if(s.begin(), s.end(), 
			[](unsigned char c) { return !std::isdigit(c); }) == s.end();
	}
	std::string text_;
};

class FormulaImpl : public Impl {
public:
	FormulaImpl(string text, const SheetInterface& sheet)
		: sheet_(sheet)
	{
		value_ = ParseFormula(text);
	}
	~FormulaImpl() = default;

	Value GetValue() const override {
		if (!cached_value_.has_value()) {
			auto val = value_->Evaluate(sheet_);
			if (holds_alternative<double>(val)) {
				cached_value_ = get<double>(val);
			} else {
				cached_value_ = get<FormulaError>(val);
			}
		}
		return cached_value_.value();
	}
	string GetText() const override {
		return FORMULA_SIGN + value_->GetExpression();
	};
	std::vector<Position> GetReferencedCells() const override {
		return value_->GetReferencedCells();
	};
	void InvalidateCache() {
		cached_value_ = nullopt;
	}

private:
	const SheetInterface& sheet_;
	unique_ptr<FormulaInterface> value_;
	mutable std::optional<CellInterface::Value> cached_value_;
};

}

Cell::Cell(const SheetInterface& sheet)
	:	sheet_(sheet),
		impl_(make_unique<EmptyImpl>())
{
}

Cell::~Cell() {
}

void Cell::Set(const std::string text) {
	if (text.empty()) {
		impl_ = make_unique<EmptyImpl>();
	} else if (IsFormula(text)) {
		impl_ = make_unique<FormulaImpl>(text.substr(1), sheet_);
	} else {
		impl_ = make_unique<TextImpl>(text);
	}
}

void Cell::Clear() {
}

Cell::Value Cell::GetValue() const {
	return impl_->GetValue();
}

std::string Cell::GetText() const {
	return impl_->GetText();
}

bool Cell::IsFormula(const std::string& text) {
	if (text[0] == FORMULA_SIGN && text.size() > 1) {
		return true;
	}
	return false;
}

std::vector<Position> Cell::GetReferencedCells() const {
	return impl_->GetReferencedCells();
}

// Node
void Cell::AddOutPos(const Position& pos) {
	out_poses_.insert(pos);
}

void Cell::DelOutPos(const Position& pos) {
	out_poses_.erase(pos);
}

void Cell::AddInPos(const Position& pos) {
	in_poses_.insert(pos);
}

void Cell::DelInPos(const Position& pos) {
	in_poses_.erase(pos);
}

std::set<Position>& Cell::GetOutPoses() {
	return out_poses_;
}

std::set<Position>& Cell::GetInPoses() {
	return in_poses_;
}

void Cell::InvalidateCache() {
	FormulaImpl* formula_ptr = dynamic_cast<FormulaImpl*>(impl_.get());
	if (formula_ptr) {
		formula_ptr->InvalidateCache();
	}
}

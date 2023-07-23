#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    std::string prev_value;
    SetNewValue(pos, text, &prev_value);
    std::vector<Position> refs = GetCell(pos)->GetReferencedCells();
    if (!refs.empty()) {
        CheckReferencies(pos, refs, &prev_value);
    }
    // инвалидировать зависимые €чейки
    InvalidateCache(pos);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetCellInternal(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return const_cast<CellInterface*>(GetCellInternal(pos));
}

void Sheet::ClearCell(Position pos) {
    if (IsPosValid(pos)) {
        if (cells_.count(pos)) {
            DeleteRefsToThisPosition(pos);
            cells_.erase(pos);
            if (pos.row == size_.rows - 1) {
                FindnSetMaxRow();
            }
            if (pos.col == size_.cols - 1) {
                FindnSetMaxCol();
            }
        }
    }
}

Size Sheet::GetPrintableSize() const {
    return size_;
}

void Sheet::PrintValues(std::ostream& output) const {
    auto printer = [this](std::ostream& output, int row, int col) {
        std::visit(CellValuePrinter{ output }, this->cells_.at({ row, col })->GetValue());
    };
    DoForEachCell(output, printer);
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto printer = [this](std::ostream& output, int row, int col) {
        output << this->cells_.at({ row, col })->GetText();
    };
    DoForEachCell(output, printer);
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

// private section =================================================

const CellInterface* Sheet::GetCellInternal(Position pos) const {
    if (IsPosValid(pos)) {
        if (cells_.count(pos)) {
            return cells_.at(pos).get();
        }
    }
    return nullptr;
}

bool Sheet::IsPosValid(Position pos) const {
    if (!pos.IsValid())
        throw InvalidPositionException("Fatal error: Invalid position");
    return true;
}

void Sheet::SetIfMaxPos(const Position& pos) {
    if (size_.rows <= pos.row) {
        size_.rows = pos.row + 1;
    }
    if (size_.cols <= pos.col) {
        size_.cols = pos.col + 1;
    }
}

void Sheet::FindnSetMaxRow() {
    for (int row = size_.rows-1; row >= 0; --row) {
        for (int col = size_.cols-1; col >= 0; --col) {
            if (cells_.count({ row, col })) {
                size_.rows = row+1;
                return;
            }
        }
    }
    size_.rows = 0;
}

void Sheet::FindnSetMaxCol() {
    for (int col = size_.cols - 1; col >= 0; --col) {
        for (int row = size_.rows - 1; row >= 0; --row) {
            if (cells_.count({ row, col })) {
                size_.cols = col+1;
                return;
            }
        }
    }
    size_.cols = 0;
}

// Graph
void Sheet::AddEdgesToNode(const Position& from, const std::vector<Position>& tos) {
    for (const Position& to : tos) {
        if (!cells_.count(to)) {
            cells_[to] = std::make_unique<Cell>(*this);
        }
        cells_[from]->AddOutPos(to);
        cells_[to]->AddInPos(from);
    }
}

void Sheet::CheckCyclicDependencies(Position start_pos, Position target) {
    static int level_ = 0;
    static Position start_pos_;
    static std::set<Position> checked_;
    if (level_ == 0) {
        start_pos_ = start_pos;
        if (start_pos_ == target) {
            throw CircularDependencyException("");
        }
    }

    if (!cells_.count(target)) {
        return;
    }

    std::set<Position>& refs = cells_.at(target)->GetOutPoses();

    if (refs.empty())
        return;

    for (Position pos : refs) {
        if (start_pos_ == pos) {
            level_ = 0;
            checked_.clear();
            throw CircularDependencyException("Found circular dependency!");
        }
        if (checked_.count(pos)) {
            continue;
        }
        ++level_;
        CheckCyclicDependencies(pos, pos);
        checked_.insert(pos);
        --level_;
    }
    level_ = 0;
    checked_.clear();
}

void Sheet::InvalidateCache(Position start_pos) {
    static int level_ = 0;
    static Position start_pos_;
    static std::set<Position> checked_;
    if (level_ == 0) {
        start_pos_ = start_pos;
    }

    std::set<Position>& refs = cells_.at(start_pos)->GetInPoses();

    if (refs.empty()) {
        return;
    }

    for (Position pos : refs) {
        if (checked_.count(pos)) {
            continue;
        }
        ++level_;
        InvalidateCache(pos);
        cells_.at(pos)->InvalidateCache();
        checked_.insert(pos);
        --level_;
    }
    level_ = 0;
    checked_.clear();
}

void Sheet::DeleteRefsToThisPosition(Position start_pos) {
    std::set<Position>& refs = cells_.at(start_pos)->GetInPoses();

    if (refs.empty()) {
        return;
    }

    for (Position pos : refs) {
        cells_.at(pos)->DelOutPos(start_pos);
    }
}

void Sheet::SetNewValue(Position pos, std::string text, std::string* prev_value) {
    if (IsPosValid(pos)) {
        SetIfMaxPos(pos);
        if (!GetCell(pos)) {
            cells_[pos] = std::make_unique<Cell>(*this);
            cells_[pos]->Set(text);
        }
        else {
            if (prev_value) {
                *prev_value = cells_[pos]->GetText();
            }
            cells_[pos]->Set(text);
        }
    }
}

void Sheet::CheckReferencies(Position pos, const std::vector<Position>& refs, std::string* prev_value) {
    const Position& start = pos;
    const std::vector<Position>& targets = refs;
    try {
        for (const Position& target : targets) {
            CheckCyclicDependencies(start, target);
        }
        AddEdgesToNode(start, targets);
    }
    catch (const CircularDependencyException& err) {
        if (prev_value) {
            cells_[pos]->Set(*prev_value);
        }
        throw err;
    }
}

#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <memory>
#include <unordered_map>
#include <map>
#include <deque>

/* Интерфейс, добавляющий поддержку хранения зависимостей
   между ячейками таблицы в виде графа */
class Graph {
public:
    /* Добавляет ребра к узлам графа (исходящие и входящие зависимости) */
    virtual void AddEdgesToNode(const Position& from, const std::vector<Position>& to) = 0;
    /* Обходит граф исходящих зависимостей для обнаружения 
       циклической зависимости относительно текущего узла (позиции start_pos) */
    virtual void CheckCyclicDependencies(Position start_pos, Position target) = 0;
    /* Обходит граф входящих зависимостей для инвалидации кэша
       при изменении значения текущего узла (ячейки) */
    virtual void InvalidateCache(Position start_pos) = 0;
    virtual void DeleteRefsToThisPosition(Position start_pos) = 0;
};

class Sheet : public SheetInterface, public Graph {
private:
    struct Hasher {
        size_t operator()(const Position& p) const {
            return std::hash<int>{}(p.row) + 31 * std::hash<int>{}(p.col);
        }
    };

public:
    using SheetCell = std::unique_ptr<Cell>;
    using SheetCells = std::unordered_map<Position, SheetCell, Hasher>;
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;

private:
    template<typename F>
    void DoForEachCell(std::ostream& output, F printer) const;
    const CellInterface* GetCellInternal(Position pos) const;
    bool IsPosValid(Position pos) const;
    void SetIfMaxPos(const Position& pos);
    void FindnSetMaxRow();
    void FindnSetMaxCol();
    void SetNewValue(Position pos, std::string text, std::string* prev_value = nullptr );
    void CheckReferencies(Position pos, const std::vector<Position>& refs, std::string* prev_value = nullptr);

    SheetCells cells_;
    Size size_;

    // Переопределенные методы класса Graph
    void AddEdgesToNode(const Position& from, const std::vector<Position>& to) override;
    void CheckCyclicDependencies(Position start_pos, Position target) override;
    void InvalidateCache(Position start_pos) override;
    void DeleteRefsToThisPosition(Position start_pos) override;
};

template<typename F>
void Sheet::DoForEachCell(std::ostream& output, F printer) const {
    if (GetPrintableSize() == Size{ 0, 0 }) {
        return;
    }
    for (int row = 0; row < size_.rows; ++row) {
        for (int col = 0; col < size_.cols; ++col) {
            if (cells_.count({ row, col })) {
                printer(output, row, col);
            }
            col != size_.cols-1 ? output << '\t' : output << '\n';
        }
    }
}

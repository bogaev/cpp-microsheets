#pragma once

#include "common.h"
#include "formula.h"

#include <iostream>
#include <optional>
#include <set>

class Impl;

/* Интерфейс, добавляющий поддержку хранения исходящих и входящих
   зависимостей для данного узла */
class Node {
public:
    /* Добавляет исходящую зависимость для текущего узла */
    virtual void AddOutPos(const Position& pos) = 0;
    virtual void DelOutPos(const Position& pos) = 0;
    /* Добавляет входящую зависимость для текущего узла */
    virtual void AddInPos(const Position& pos) = 0;
    virtual void DelInPos(const Position& pos) = 0;
    /* Возвращает исходящие зависимости текущего узла */
    virtual std::set<Position>& GetOutPoses() = 0;
    /* Возвращает входящие зависимости текущего узла */
    virtual std::set<Position>& GetInPoses() = 0;
};

class Cell : public CellInterface, public Node {
public:
    explicit Cell(const SheetInterface& sheet);
	~Cell();

	void Set(const std::string text);
	void Clear();

	Value GetValue() const override;
	std::string GetText() const override;

    std::vector<Position> GetReferencedCells() const override;

    // Переоределенные методы класса Node
    void AddOutPos(const Position& pos) override;
    void DelOutPos(const Position& pos) override;
    void AddInPos(const Position& pos) override;
    void DelInPos(const Position& pos) override;
    std::set<Position>& GetOutPoses() override;
    std::set<Position>& GetInPoses() override;
    void InvalidateCache();

private:
    bool IsFormula(const std::string& text);

    const SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;

    // Данные узла
    /* Исходящие зависимости данного узла */
    std::set<Position> out_poses_;
    /* Входящие зависимости данного узла */
    std::set<Position> in_poses_;
};

struct CellValuePrinter {
    std::ostream& output;
    std::ostream& operator()(const std::string str) const {
        return output << str;
    }
    std::ostream& operator()(double num) const {
        return output << num;
    }
    std::ostream& operator()(const FormulaError& error) const {
        return output << error;
    }
};

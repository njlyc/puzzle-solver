#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <locale>
#include <io.h>
#include <fcntl.h>

using Pos = std::pair<int, int>;
using Shape = std::vector<Pos>;

void standardize(Shape &positions)
{
    auto minX = std::min_element(positions.begin(), positions.end(),
                                 [](Pos const &lhs, Pos const &rhs)
                                 { return lhs.first < rhs.first; })
                    ->first,
         minY = std::min_element(positions.begin(), positions.end(),
                                 [](Pos const &lhs, Pos const &rhs)
                                 { return lhs.second < rhs.second; })
                    ->second;
    std::sort(positions.begin(), positions.end(),
              [](Pos const &lhs, Pos const &rhs)
              { return (lhs.first < rhs.first) ? true : (lhs.first == rhs.first && lhs.second < rhs.second); });
    std::for_each(positions.begin(), positions.end(),
                  [minX, minY](Pos &pos)
                  {
                      pos.first -= minX;
                      pos.second -= minY;
                  });
}

Shape linearTransform(Shape const &positions, std::vector<int> const &mat)
{
    Shape newPositions;
    std::transform(positions.begin(), positions.end(), std::back_inserter(newPositions),
                   [&mat](Pos const &pos)
                   { return Pos{pos.first * mat[0] + pos.second * mat[1], pos.first * mat[2] + pos.second * mat[3]}; });
    standardize(newPositions);
    return newPositions;
}

class Block
{
public:
    Block(std::initializer_list<Pos> const &arg) : unused(true)
    {
        Shape initial{arg};
        standardize(initial);
        static std::vector<std::vector<int>> transformList{
            {1, 0, 0, 1},
            {0, -1, 1, 0},
            {-1, 0, 0, -1},
            {0, 1, -1, 0},
            {-1, 0, 0, 1},
            {0, 1, 1, 0},
            {1, 0, 0, -1},
            {0, -1, -1, 0}};
        std::transform(transformList.begin(), transformList.end(), std::back_inserter(shapes),
                       [&initial](std::vector<int> const &arg)
                       { return linearTransform(initial, arg); });
        std::sort(shapes.begin(), shapes.end());
        shapes.erase(std::unique(shapes.begin(), shapes.end()), shapes.end());

        std::for_each(shapes.begin(), shapes.end(),
                      [](Shape &shape)
                      {
                          auto bias = shape[0].second;
                          std::for_each(shape.begin(), shape.end(), [bias](Pos &pos)
                                        { pos.second -= bias; });
                      });
    }
    std::vector<Shape> shapes;
    bool unused;
};

class Board
{
public:
    Board(std::vector<std::vector<int>> const &matrix) : data(matrix), rows(matrix.size()), cols(matrix[0].size())
    {
        addBorder();
    }
    void addBorder(int val = -1)
    {
        std::for_each(data.begin(), data.end(), [val](std::vector<int> &row)
                      {
                          row.insert(row.begin(), val);
                          row.push_back(val);
                      });
        std::vector<int> fullRow(cols + 2, -1);
        data.insert(data.begin(), fullRow);
        data.push_back(fullRow);
        rows += 2;
        cols += 2;
    }

    void print()
    {
        enum Dir : int
        {
            Up = 1,
            Left = 1 << 1,
            Down = 1 << 2,
            Right = 1 << 3
        };

        static std::map<int, std::wstring> charMapping{
            {0, L" "},
            {Dir::Up + Dir::Right, L"└"},
            {Dir::Up + Dir::Left, L"┘"},
            {Dir::Up + Dir::Down, L"│"},
            {Dir::Down + Dir::Right, L"┌"},
            {Dir::Left + Dir::Down, L"┐"},
            {Dir::Left + Dir::Right, L"─"},
            {Dir::Up + Dir::Left + Dir::Down, L"┤"},
            {Dir::Up + Dir::Left + Dir::Right, L"┴"},
            {Dir::Up + Dir::Down + Dir::Right, L"├"},
            {Dir::Left + Dir::Down + Dir::Right, L"┬"},
            {Dir::Up + Dir::Down + Dir::Right + Dir::Left, L"┼"}};

        for (auto i = 0; i < rows - 1; ++i)
        {
            for (auto j = 0; j < cols - 1; ++j)
            {
                int type = 0;
                if (data[i][j] != data[i][j + 1])
                    type += Dir::Up;
                if (data[i][j] != data[i + 1][j])
                    type += Dir::Left;
                if (data[i + 1][j] != data[i + 1][j + 1])
                    type += Dir::Down;
                if (data[i][j + 1] != data[i + 1][j + 1])
                    type += Dir::Right;
                std::wcout << charMapping[type] << L" ";
            }
            std::wcout << "\n";
        }
    }
    std::vector<std::vector<int>> data;
    int rows, cols;
};

class Puzzle
{
public:
    Puzzle(Board const &argBoard, std::vector<Block> argBlocks) : board(argBoard), blocks(argBlocks)
    {
    }

    int solve(bool show)
    {
        solution = 0;
        ifPrint = show;

        dfs(1, 1, 1);

        return solution;
    }

    void dfs(int i, int j, int k)
    {
        while (i != board.rows && board.data[i][j] != 0)
        {
            if (++j == board.cols)
            {
                ++i;
                j = 0;
            }
        }
        if (i == board.rows)
        {
            if (ifPrint)
                board.print();
            solution += 1;
            return;
        }
        for (auto &block : blocks)
        {
            if (!block.unused)
                continue;
            for (auto &shape : block.shapes)
            {
                if (std::any_of(shape.begin(), shape.end(), [i, j, this](Pos const &pos)
                                {
                                    auto newI = i + pos.first, newJ = j + pos.second;
                                    return newI < 1 || board.rows <= newI || newJ < 1 || board.cols <= newJ || board.data[newI][newJ] != 0;
                                }))
                    continue;
                std::for_each(shape.begin(), shape.end(), [i, j, k, this](Pos const &pos)
                              {
                                  auto newI = i + pos.first, newJ = j + pos.second;
                                  board.data[newI][newJ] = k;
                              });
                block.unused = false;
                dfs(i, j, k + 1);
                block.unused = true;
                std::for_each(shape.begin(), shape.end(), [i, j, this](Pos const &pos)
                              {
                                  auto newI = i + pos.first, newJ = j + pos.second;
                                  board.data[newI][newJ] = 0;
                              });
            }
        }
    }
    Board board;
    std::vector<Block> blocks;
    int solution;
    bool ifPrint;
};

class CalendarPuzzle : public Puzzle
{
public:
    CalendarPuzzle() : Puzzle(theBoard, theBlocks)
    {
        for (auto month = 1; month <= 12; ++month)
        {
            monthMapping[month] = {(month - 1) / 6 + 1, (month - 1) % 6 + 1};
        }
        for (auto day = 1; day <= 31; ++day)
        {
            dayMapping[day] = {(day - 1) / 7 + 3, (day - 1) % 7 + 1};
        }
    }
    int solve(int month, int day, bool show)
    {
        for (auto [x, y] : {monthMapping[month], dayMapping[day]})
            board.data[x][y] = -1;

        return Puzzle::solve(show);
        for (auto [x, y] : {monthMapping[month], dayMapping[day]})
            board.data[x][y] = 0;
    }
    static std::vector<Block> theBlocks;
    static std::vector<std::vector<int>> theBoard;
    std::map<int, Pos> monthMapping, dayMapping;
};

std::vector<Block> CalendarPuzzle::theBlocks{
    {{0, 0}, {0, 1}, {1, 0}, {1, 1}, {2, 0}, {2, 1}},
    {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {1, 3}},
    {{0, 0}, {-1, 0}, {-1, -1}, {1, 0}, {1, 1}},
    {{0, 0}, {0, 1}, {0, 2}, {1, 1}, {1, 2}},
    {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {2, 0}},
    {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 2}},
    {{0, 0}, {-1, 0}, {-1, -1}, {1, 0}, {1, -1}},
    {{0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}}};

std::vector<std::vector<int>> CalendarPuzzle::theBoard{
    {0, 0, 0, 0, 0, 0, -1},
    {0, 0, 0, 0, 0, 0, -1},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, -1, -1, -1, -1}};

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    CalendarPuzzle sol;
    int m, d;
    std::wcin >> m >> d;
    auto ans = sol.solve(m, d, true);
    std::wcout << ans << L" Solutions found\n";
    system("pause");
}
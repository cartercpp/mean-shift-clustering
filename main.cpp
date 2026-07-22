#include <iostream>
#include <format>
#include <utility>
#include <array>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <stop_token>
#include <cmath>

int main()
{
    constexpr int rows = 80,
                  columns = 120,
                  pointCount = rows * columns / 10;

    constexpr double bandwidth = columns / 10.0;

    char grid[rows][columns];

    auto resetGrid = [&grid]()
    {
        for (int row = 0; row < rows; ++row)
            for (int column = 0; column < columns; ++column)
                grid[row][column] = ' ';
    };

    std::array<std::pair<double, double>, pointCount> points{};

    std::random_device rd;
    std::uniform_real_distribution<double> xDist(0, columns - 1),
                                           yDist(0, rows - 1);

    auto generatePoints = [&]()
    {
        for (auto& [xRef, yRef] : points)
        {
            xRef = xDist(rd);
            yRef = yDist(rd);
        }
    };

    auto moveCursor = [](int row, int column)
    {
        std::cout << std::format("\033[{};{}H", row + 1, column + 1);
    };

    std::cout << "\033[?25l";
    std::cout << "\033[2J";

    std::jthread thr{[&](std::stop_token st)
    {
        while (!st.stop_requested())
        {
            generatePoints();

            for (int epoch = 0; epoch < 25; ++epoch)
            {
                resetGrid();

                std::array<std::pair<double, double>, pointCount> newPoints{};

                for (int i = 0; i < pointCount; ++i)
                {
                    const auto [x, y] = points[i];

                    const int row = rows - 1 - static_cast<int>(y),
                              column = static_cast<int>(x);
                    grid[row][column] = 'o';

                    std::pair<double, double> newPoint{};
                    int divideBy = 0;

                    for (int i2 = 0; i2 < pointCount; ++i2)
                    {
                        const auto [x2, y2] = points[i2];

                        const double distance = std::sqrt((x - x2) * (x - x2) + (y - y2) * (y - y2));

                        if (distance < bandwidth)
                        {
                            newPoint.first += x2;
                            newPoint.second += y2;
                            ++divideBy;
                        }
                    }

                    if (divideBy != 0)
                    {
                        newPoint.first /= divideBy;
                        newPoint.second /= divideBy;
                    }
                    else
                        newPoint = points[i];

                    newPoints[i] = newPoint;
                }

                moveCursor(0, 0);
                for (int row = 0; row < rows; ++row)
                {
                    for (int column = 0; column < columns; ++column)
                        std::cout << grid[row][column];

                    std::cout << '\n';
                }

                std::swap(points, newPoints);

                if (epoch > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                else
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }};
    std::cin.get();

    std::cout << "\033[?25h";
}
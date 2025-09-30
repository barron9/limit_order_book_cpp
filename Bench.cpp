#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <string>
#include <cstdint>
#include <benchmark/benchmark.h>

enum class Side { Buy, Sell };

struct Order {
    uint64_t id;
    double price;
    uint32_t quantity;
    Side side;
};
template <typename T>
class TrackingAllocator {
public:
    using value_type = T;

    static inline size_t total_allocated = 0;
    static inline size_t total_deallocated = 0;

    TrackingAllocator() = default;

    template <class U>
    constexpr TrackingAllocator(const TrackingAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        total_allocated += n * sizeof(T);
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept {
    total_allocated -= n * sizeof(T);
    total_deallocated += n * sizeof(T); // ← Add this
    ::operator delete(p);
    }  
};

template <typename T>
size_t TrackingAllocator<T>::total_allocated = 0;
template <typename Alloc = std::allocator<Order>>
struct PriceLevel {
    double price;
    std::deque<Order, Alloc> orders;

    bool operator<(const PriceLevel& other) const {
        return price < other.price;
    }
};

class MatchingEngine {
public:
    void process_order(double price, uint32_t qty, Side side) {
        Order incoming{next_id++, price, qty, side};
        auto& book = (side == Side::Buy) ? sell_levels : buy_levels;

        auto price_matches = [&](double book_price) {
            return (side == Side::Buy) ? (incoming.price >= book_price)
                                       : (incoming.price <= book_price);
        };

        while (incoming.quantity > 0 && !book.empty()) {
            PriceLevel<TrackingAllocator<Order>>& level = book.front();

            if (!price_matches(level.price)) break;

            while (!level.orders.empty() && incoming.quantity > 0) {
                Order& resting = level.orders.front();
                uint32_t traded_qty = std::min(incoming.quantity, resting.quantity);

                std::string trade = (side == Side::Buy ? "BUY " : "SELL ") +
                                    std::to_string(traded_qty) + " @ " +
                                    std::to_string(level.price);
                trades.push_back(trade);

                incoming.quantity -= traded_qty;
                resting.quantity -= traded_qty;

                if (resting.quantity == 0) {
                    level.orders.pop_front();
                }
            }

            if (level.orders.empty()) {
                book.erase(book.begin());
            }
        }

        if (incoming.quantity > 0) {
            add_order(incoming.price, incoming.quantity, side);
        }
    }

    void add_order(double price, uint32_t qty, Side side) {
        Order order{next_id++, price, qty, side};
        auto& book = (side == Side::Buy) ? buy_levels : sell_levels;

        auto it = std::find_if(book.begin(), book.end(),
            [price](const PriceLevel<TrackingAllocator<Order>>& pl) { return pl.price == price; });

        if (it != book.end()) {
            it->orders.push_back(order);
        } else {
            PriceLevel<TrackingAllocator<Order>> new_level{price, {order}};
            book.push_back(new_level);
            sort_book(book, side);
        }
    }

    void sort_book(std::vector<PriceLevel<TrackingAllocator<Order>>>& book, Side side) {
        if (side == Side::Buy) {
            std::sort(book.begin(), book.end(),
                      [](const PriceLevel<TrackingAllocator<Order>>& a, const PriceLevel<TrackingAllocator<Order>>& b) {
                          return a.price > b.price; 
                      });
        } else {
            std::sort(book.begin(), book.end(),
                      [](const PriceLevel<TrackingAllocator<Order>>& a, const PriceLevel<TrackingAllocator<Order>>& b) {
                          return a.price < b.price; 
                      });
        }
    }

    void print_book(Side side) const {
        const auto& book = (side == Side::Buy) ? buy_levels : sell_levels;
        std::cout << (side == Side::Buy ? "Buy Book:\n" : "Sell Book:\n");

        for (const auto& level : book) {
            uint32_t total_qty = 0;
            for (const auto& o : level.orders) total_qty += o.quantity;
            std::cout << "Price: " << level.price << ", Total Qty: " << total_qty << "\n";
        }
    }

    void print_trades() const {
        std::cout << "\nTrades:\n";
        for (const auto& t : trades) {
            std::cout << t << "\n";
        }
    }

private:
    std::vector<PriceLevel<TrackingAllocator<Order>>> buy_levels;
    std::vector<PriceLevel<TrackingAllocator<Order>>> sell_levels;
    std::vector<std::string> trades;
    uint64_t next_id = 1;
};


static void BM_ProcessOrder_Buy(benchmark::State& state) {
    MatchingEngine engine;

    for (auto _ : state) {
       engine.process_order(101.0, 20, Side::Buy); 
       //engine.add_order(101.0, 20, Side::Buy); 
    }
}
BENCHMARK(BM_ProcessOrder_Buy);


static void BM_AddOrder_Buy(benchmark::State& state) {
    MatchingEngine engine;

    for (auto _ : state) {
       engine.add_order(101.0, 20, Side::Buy); 
       //engine.add_order(101.0, 20, Side::Buy); 
    }
}
BENCHMARK(BM_AddOrder_Buy);


// ----------- Main Entry Point -----------
//BENCHMARK_MAIN();
int main(int argc, char** argv){
    MatchingEngine engine;

    constexpr int iterations = 100000;

    for (int i = 0; i < iterations; ++i) {
        engine.process_order(101.0, 20, Side::Buy);
    }

    for (int i = 0; i < iterations; ++i) {
        engine.process_order(101.0, 20, Side::Sell);
    }

    size_t allocated = TrackingAllocator<Order>::total_allocated;
    size_t deallocated = TrackingAllocator<Order>::total_deallocated;
    long long net_memory = static_cast<long long>(allocated) - static_cast<long long>(deallocated);

    std::cout << "Total allocated: " << allocated << " bytes\n";
    std::cout << "Total deallocated: " << deallocated << " bytes\n";
    std::cout << "Net memory usage: " << net_memory << " bytes\n";
     // Run Google Benchmark benchmarks
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}

//cl /EHsc /O2 /MD /DBENCHMARK_STATIC_DEFINE Bench.cpp /I../benchmark/include  ..\benchmark\build\src\Release\benchmark.lib  ..\benchmark\build\src\Release\benchmark_main.lib  /link shlwapi.lib advapi32.lib
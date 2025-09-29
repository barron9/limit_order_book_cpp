# limit_order_book_cpp

### System Info
- **CPU**: 24 × 2800 MHz
- **CPU Caches**:
  - L1 Data: 32 KiB ×12
  - L1 Instruction: 32 KiB ×12
  - L2 Unified: 512 KiB ×12
  - L3 Unified: 16,384 KiB ×4

---

## 📊 Benchmark Results

| Benchmark               | Time    | CPU     | Iterations |
|-------------------------|---------|---------|------------|
| `BM_ProcessOrder_Buy`   | 72.1 ns | 69.8 ns | 11,200,000 |
| `BM_AddOrder_Buy`       | 65.5 ns | 61.0 ns | 8,960,000  |
| `BM_MultiProcessOrder`  | 149 ns  | 141 ns  | 4,977,778  |

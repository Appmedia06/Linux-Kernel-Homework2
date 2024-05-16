import matplotlib.pyplot as plt

# 兩組數組
data1 = [0.03153, 0.01125, 0.009509]
data2 = [0.31873, 0.11808, 0.11707]

# x 軸的數據，這裡使用了索引值
x = ["non_rand", "rand_pivot", "rand_threeMedian_pivot"]

plt.figure(figsize=(10, 6))

# 畫出折線圖
plt.plot(x, data1, label='Normal case', marker='o')  # 第一組數據
plt.plot(x, data2, label='Worst case', marker='s')  # 第二組數據

# 添加標籤和標題
plt.xlabel('')
plt.ylabel('Sort time(sec)')
plt.title('Quick Sort perfomance comparsion')

# 添加背景網格
# plt.grid(True)

# 添加圖例
plt.legend()

# 顯示圖形
plt.show()


# BÀI TẬP LỚN HỆ NHÚNG: CẢM BIẾN ĐO NHỊP TIM VÀ SPO2 

## A. Giới thiệu
Tính năng chính: Đo nhịp tim (BPM) và nồng độ SPO2 trong máu, cảnh báo khi chỉ số bất thường

## B. Hướng dẫn sử dụng
- Bước 1: Khởi động ứng dụng, màn hình hiển thị power up  
- Bước 2: Sau khi power up thành công, màn hình hiển thị "Put your finger". Đặt ngón tay lên.
- Bước 3: Ứng dụng sẽ đo và hiển thị chỉ số nhịp tim và nồng độ SPO2 
- Bước 4: Bỏ ngón tay ra, ứng dụng sẽ dừng đo.

## C. Danh sách link kiện
<strong>1. ESP32</strong>
<p>Số lượng 1</p>
<img src="/images/esp32.jpg" alt="esp32" />
<a href="https://linhkienvietnam.vn/module-node-mcu-32s-esp32-devkitc-dung-module-esp-wroom-32">Link mua sản phẩm</a>
<br>
<strong>2. MAX30102</strong>
<p>Số lượng 1</p>
<img src="/images/max30102.jpg" alt="max30102" />
<a href="https://vn.shp.ee/DqZJcef">Link mua sản phẩm</a>
<br>
<strong>3. OLED 0.96</strong>
<p>Số lượng 1</p>
<img src="/images/oled.jpg" alt="oled" />
<a href="https://nshopvn.com/product/man-hinh-oled-0-96-inch-giao-tiep-spi/">Link mua sản phẩm</a>
<br>
<strong>4. Vibration Motor</strong>
<p>Số lượng 1</p>
<img src="/images/vm.jpg" alt="vm" />
<a href="https://linhkien024.vn/module-dong-co-rung-dien-thoai-3-5vdc-bao-rung">Link mua sản phẩm</a>
<br>

## D. Sơ đồ nguyên lý
![image](https://github.com/Starry-Nightt/SpO2/assets/95482961/a6749a58-9013-4acc-ba9f-d796ffd5923a)

## E. Thiết kế phần mềm
Khi thiết bị khởi động, phần mềm sẽ kiểm tra cảm biến MAX30102 có kết nối với vi điều khiển không. Nếu có lỗi xảy ra, trên màn hình OLED xuất hiện thông báo “MAX30102 was not found. Please check wiring/power.”. Phần mềm tiến hành đọc 100 mẫu giá trị đầu tiên và lưu trữ giá trị của độ sáng đèn LED hồng ngoại vào biến `irBuffer` và giá trị độ sáng đèn LED đỏ tại biến `redBuffer`, số 100 được lưu ở biến  `bufferLength`. 

Phần mềm tính toán giá trị nhịp tim và spO2 thông qua 2 giá trị đo được và hiển thị giá trị dữ liệu đọc được lên trên màn OLED. Nếu cảm biến chưa đọc được giá trị, trên màn hình OLED sẽ hiển thị lời nhắc “Put your finger!”. 

Theo thực tế, nếu nồng độ spO2 hiện tại dưới 94% thì màn hình hiển thị “Low SPO2”. Để đánh giá nhịp tim cao hay thấp, nhóm đã đánh giá thông qua giá trị nhịp tim trung bình và giá trị nhịp tim hiện tại của người dùng. Giá trị nhịp tim trung bình (được lưu trữ ở biến `avgHeartRate`) được tính toán thông qua 100 mẫu giá trị mới nhất mà cảm biến đo được. Trong trường hợp nhịp tim trung bình nhỏ hơn 80 BMP: hiển thị trên màn hình nhịp tim thấp nếu nhịp tim hiện tại nhỏ hơn 45 BMP, nhịp tim cao nếu nhịp tim hiện tại lớn hơn 105 BMP. Trong trường hợp còn lại, khi nhịp tim trung bình lớn hơn 80 thì màn hình sẽ hiển thị nhịp tim thấp nếu nhịp tim hiện tại nhỏ hơn 55 BMP, nhịp tim cao nếu nhịp tim hiện tại lớn hơn 110 BMP. 

Trong quá trình lấy mẫu và tính toán nhịp tim, chỉ số spO2, nếu bắt gặp các giá trị bất thường như nêu trên thì còi báo động sẽ kêu để cảnh báo nhờ đặt ngưỡng giá trị là `HIGH`.

## F. Tác giả
| Họ tên  | MSSV  | Công việc    |
| :---:   | :---: | :---:        |
| Đặng Minh Tiến | 20204794   | Lắp thiết bị và code          |
| Trần Thanh Hiền | 20204744   | Làm báo cáo         |
| Hà Anh Vũ | 20204863   | Lắp thiết bị và kiểm thử        |
| Hồ Sỹ Vinh | 20204863   |         |

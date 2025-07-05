# Đồ án Môn học: Mạng Cảm biến không dây để đo nhiệt độ
# Course Project: Wireless Sensor Network for Temperature Monitoring

Đây là đồ án đồng đội cho môn học "Mạng Cảm biến không dây" tại ĐH Bách Khoa Hà Nội. Mục tiêu của dự án là xây dựng một hệ thống hoàn chỉnh với các node cảm biến dùng pin để đo nhiệt độ tại các đầu máy và truyền dữ liệu không dây về một gateway trung tâm để giám sát.

## Kiến trúc hệ thống

Hệ thống được thiết kế theo mô hình mạng hình sao (Star Network), bao gồm 2 thành phần chính:
1.  **Sensor Node (Nút cảm biến):** Sử dụng vi điều khiển ESP32 và cảm biến nhiệt độ LM35, được cấp nguồn bằng pin. Các node này có nhiệm vụ đo nhiệt độ và gửi dữ liệu về Gateway thông qua giao thức **ESP-NOW**.
2.  **Gateway:** Sử dụng một vi điều khiển ESP32 khác, có vai trò nhận dữ liệu từ tất cả các Sensor Node qua ESP-NOW. Sau đó, Gateway kết nối vào mạng Wi-Fi và đẩy toàn bộ dữ liệu lên nền tảng **ThingsBoard** qua giao thức **MQTT** để giám sát thời gian thực.

## Công nghệ & Kỹ năng nổi bật

-   **Thiết kế Phần cứng:** Thiết kế sơ đồ nguyên lý (Schematic) và mạch in (PCB) bằng **Altium Designer**.
-   **Mạng không dây:** Sử dụng giao thức **ESP-NOW** cho mạng cảm biến tiêu thụ năng lượng thấp.
-   **Lập trình Nhúng:** Lập trình vi điều khiển **ESP32** bằng ngôn ngữ C++.
-   **Giao thức IoT:** **MQTT** để giao tiếp với cloud platform.
-   **Nền tảng IoT:** **ThingsBoard** để trực quan hóa dữ liệu và quản lý thiết bị.

## Phân chia công việc & Đóng góp của cá nhân

Đây là một dự án hợp tác nơi mỗi thành viên có một vai trò chuyên môn riêng. **Trách nhiệm chính của tôi trong dự án này là phát triển toàn bộ phần cứng cho các node cảm biến.**

* **Thiết kế Phần cứng:** Chịu trách nhiệm thiết kế sơ đồ nguyên lý và layout mạch in (PCB) cho các node cảm biến, bao gồm cả khối nguồn và mạch xử lý.
* **Lựa chọn Linh kiện:** Nghiên cứu và lựa chọn vi điều khiển, cảm biến phù hợp với yêu cầu về dải đo và độ chính xác của dự án.
* **Thi công và Kiểm thử:** Lắp ráp, hàn linh kiện và hoàn thiện các bo mạch vật lý để kiểm thử.

*Phần firmware (lập trình ESP-NOW, MQTT) được phát triển bởi các thành viên khác trong nhóm. Mã nguồn được đưa lên repository này để cung cấp một cái nhìn hoàn chỉnh về toàn bộ dự án.*

## Cấu trúc thư mục

-   **/1_Firmware_Sensor_Node/:** Chứa code cho ESP32 hoạt động như một node cảm biến.
-   **/2_Firmware_Gateway/:** Chứa code cho ESP32 hoạt động như một gateway.
-   **/3_Hardware_Design_PCB/:** Chứa các file thiết kế phần cứng (Altium, Gerber, PDF Schematic...).

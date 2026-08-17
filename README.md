# Thần Long Item Consolidator v0.2.5

Bản sửa tập trung vào 3 lỗi thực tế của v0.2.4: **FREEZE toàn chuỗi bán**, **editor chọn đúng dòng sau khi sao chép nhiều dòng**, và **REC tự dọn lease/transaction treo**.

## 1. MAIN chạy CHUỖI CLICK BÁN ĐỒ = FREEZE ALL suốt chuỗi
BĐPT chỉ lấy `SEQUENCE LEASE` khi MAIN thật sự bước vào phase `CHUỖI CLICK BÁN ĐỒ`. Từ lúc đó đến khi dòng cuối cùng hoàn tất, tất cả acc khác bị FREEZE liên tục, kể cả thời gian delay giữa các dòng. MAIN giữ quyền chạy các bước của chính chuỗi bán.

Không còn chu kỳ `FREEZE -> click -> UNFREEZE -> delay -> FREEZE` cho từng dòng bán. BĐPT chỉ UNFREEZE khi chuỗi bán hoàn tất, click lỗi/abort, người dùng dừng acc, hoặc REC chủ động lấy quyền cấu hình.

## 2. Sửa editor sau khi SAO CHÉP nhiều dòng
Multi-select vẫn dùng để SAO CHÉP. Nhưng dòng đang chỉnh giờ được xác định bằng **focused row** (dòng người dùng vừa click), không lấy selected row đầu tiên. Vì vậy chọn một dòng bất kỳ sau khi copy nhiều dòng sẽ nạp đúng ACC/Loại/Mô tả/Delay/Lặp của dòng đó.

Đặc biệt ở workflow CON: nếu dòng đang là `MAIN #n`, đổi ACC về CON sẽ nạp lại editor đúng dòng và mở lại Mô tả + `CLICK / CHUYỂN ĐỒ` để chỉnh.

## 3. REC không còn mắc transaction/lease treo
Khi người dùng DỪNG AUTO một acc đang thuộc workflow giao dịch, BĐPT AbortTrade và nhả HOLD ngay. Khi bấm REC, nếu vẫn còn transaction hoặc click lease cũ ở tầng điều phối thì REC thu hồi/dọn trạng thái treo trước khi vào RECORDING.

REC vẫn khóa automation trong lúc ghi và sau `DỪNG REC` vẫn chuyển click tay thành các dòng tọa độ editable như v0.2.4.

## Rule giữ nguyên
- MAIN `FreeBagSpace <= 6`: bán đồ ưu tiên.
- CON chỉ giao dịch khi FULL.
- CON1 -> CON6 cố định.
- REAL INPUT vẫn đi qua BĐPT.
- Clean Route v1.5.9 route/death/revive foundation giữ nguyên.

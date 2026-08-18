# PROJECT KNOWLEDGE — Item Consolidator v0.2.4

## Central arbiter
BĐPT là cổng bắt buộc cho physical automation click. Mỗi click tự động phải qua `CoordinatorClick`; lease chuột tạo FREEZE ALL ngắn, thực hiện REAL INPUT đúng PID, nhận OK/FAIL rồi UNFREEZE. Không khôi phục background PostMessage.

## Recorder
`RecorderMode` gồm Sell / TradeMain / TradeChild. REC là input-assist, không phải runtime macro engine. Khi `coordinatorRecording_` bật, account state machines và trade scheduler không được cấp action; `CoordinatorClick` từ chối automation click. Timer `kRecordTimer=2` chạy 10 ms, poll `GetAsyncKeyState(VK_LBUTTON)` và chỉ lưu click release nằm trong game window được phép.

DỪNG REC chuyển buffer thành các `SellMacroStep` / `TradeSequenceStep` bình thường và append cuối chuỗi. Delay được suy ra từ khoảng thời gian giữa click. TradeChild nhận cả child PID và MAIN role. MAIN click dùng `FindSharedMainStepByPoint`; exact saved ClickPoint được tái sử dụng, nếu chưa có thì tạo shared MAIN step mới.

## Multi-row copy
Sell/trade step list hỗ trợ multi-select. Clipboard là in-memory cấu trúc step, không phải raw text. Paste append cuối chuỗi. Trade clipboard giữ mode để MAIN không bị dán nhầm sang CON; CON clipboard có thể tái dùng giữa các CON.

## Six click clone
`CopySixClicksFromAnotherAccount` copy từng ClickPoint hợp lệ từ client nguồn hiện đang được scan. Invalid source point không overwrite target. BaseW/BaseH được giữ để ScaleClickPoint scale sang cửa sổ đích.

## Existing business rules
MAIN sell threshold default 6; sell if freeBagSpace <= threshold. CON eligible only full. Fixed CON1..CON6 priority. MAIN/CON prep and Clean Route v1.5.9 death/revive/route foundation remain unchanged.

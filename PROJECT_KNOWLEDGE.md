# PROJECT KNOWLEDGE — v0.2.7-R1

## Baseline provenance — bắt buộc
R1 được xây **trực tiếp và duy nhất** từ v0.2.7:
- parent/head: `1308b28bd38fb044b9fceed3671820e45fb2cd23`
- exact v0.2.7 controller SHA256: `397f1cf088ce0163cdba7aea06350cc25aff8aab4627e7def9331c9f1070845f`
- R1 patch được áp sau khi rehydrate và xác minh đúng SHA trên.

**Không dùng bất kỳ source/logic nào từ v0.2.8 hoặc v0.2.9.** Các version đó không phải ancestor/input của R1. Khi sửa R1 về sau, không được “port lại” logic từ hai version đó nếu người dùng không yêu cầu rõ.

## Những thay đổi được phép trong R1

### Group repeat trong shared ACC CON workflow
`TradeSequenceStep` thêm:
- `groupId`
- `groupRepeat`

Chỉ `childTradeSequence_` global có UI gom nhóm. Chọn 2+ dòng liên tiếp, gán một group ID và số lần lặp. Runtime chạy hết block liên tiếp của group rồi lặp lại block đó cho đủ `groupRepeat`, sau đó mới tiến ra ngoài group. Row `repeat` cũ vẫn tồn tại bên trong group. Copy/paste giữ metadata group và remap group ID khi paste để tránh va chạm.

### Per-CON drain target
`AccountProfile::tradeDrainFreeSlots`, INI key `TradeDrainFreeSlots`, mặc định 6, clamp 1..90.

Quy tắc session:
1. **ENTRY GATE duy nhất:** CON chỉ được chọn khi `freeBagSpace == 0`.
2. Khi đã chọn, `tradeTxn_.childPid` giữ nguyên qua các vòng.
3. Sau mỗi chuỗi, phase `Verify` chờ số ô MAIN/CON ổn định.
4. Kết thúc nếu MAIN `freeBagSpace <= mainSellThreshold_` hoặc active CON `freeBagSpace >= tradeDrainFreeSlots`.
5. Nếu chưa đạt, chạy thêm một vòng với đúng CON hiện tại. Không re-check FULL cho child.
6. `FinishTrade` nhả HOLD; từ đó core v0.2.7 tự tiếp tục auto map/train/bán theo logic sẵn có. R1 không tạo mid-session sell flow riêng.

### Global trade rendezvous
Global profile `tradeRendezvous_` dùng INI:
- `TradeRendezvousMap`
- `TradeRendezvousX`
- `TradeRendezvousY`
- `TradeRendezvousValid`

`CaptureTradeRendezvous()` GET Map/X/Y từ snapshot acc đang chọn.

Trade-only runtime state:
- `tradeRendezvousPhase`
- `tradeRendezvousTick`
- `tradeRendezvousStopAttempts`

Luồng chuẩn bị phiên:
- giữ `tradeHeld=true` cho MAIN + active CON,
- StopPath AutoPath map/train đang có,
- dùng đúng hai click StopAuto1/StopAuto2 v0.2.7 để tắt AutoFight,
- dùng donor `HandleRobustTravel` đến `TỌA ĐỘ GIAO DỊCH`,
- chỉ khi cả hai đứng ổn định mới chạy chọn CON và shared trade workflow.

**Không dùng `trainRecoveryPhase` để đi TỌA GD.** Generic `TickAccount` vẫn bị chặn bởi `tradeHeld`; chỉ `FinishTrade/AbortTrade` mới nhả phiên.

### Auto bán FULL pushbutton
Checkbox cũ chỉ được đổi UI sang nút `AUTO BÁN FULL: BẬT/TẮT`. Nút gọi `ToggleSelectedSell()`, đảo `profile.enableSell` rồi `SaveProfile`. Không thay đổi điều kiện/scheduler bán đồ nền v0.2.7.

## Các invariant v0.2.7 phải giữ
- Active runtime trade definitions vẫn chỉ là `mainTradeSequence_` + một `childTradeSequence_` global.
- Fixed CON1 -> CON6 priority; không round-robin.
- `CHUYỂN ĐỒ` vẫn active-CON-only.
- BĐPT/`CoordinatorClick` vẫn là đường duy nhất cho physical auto click.
- Exactly 2 `SendInput(` call sites raw LEFTDOWN/LEFTUP.
- Không `tradeGlobalFreeze_`, không `roundRobinCursor_`.
- Sell sequence persistent FREEZE ALL giữ nguyên.
- DỒN ĐỒ OFF independent auto-train/sell giữ nguyên.
- REC/copy/6-click/revive/route/map/periodic confirm giữ nguyên trừ đúng các điểm nêu ở trên.

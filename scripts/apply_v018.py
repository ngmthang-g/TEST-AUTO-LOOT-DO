from pathlib import Path

root = Path('.')

# controller/probe pacing + visible version
p = root / 'src' / 'probe.cpp'
s = p.read_text(encoding='utf-8')
s = s.replace('v0.1.7', 'v0.1.8')
s = s.replace('Sleep(200);\n                if (!session.Send(Command::ClickMessageBoxConfirm, 5000, false)) continue;',
              'Sleep(500);\n                if (!session.Send(Command::ClickMessageBoxConfirm, 5000, false)) continue;')
s = s.replace('Không callback được Xác nhận trong 8 giây. Resolver v0.1.8',
              'Không callback được Xác nhận trong 20 giây. Resolver v0.1.8')
p.write_text(s, encoding='utf-8')

# protocol bump so EXE/DLL cannot be mixed silently
p = root / 'src' / 'protocol.h'
s = p.read_text(encoding='utf-8').replace('0x00010006', '0x00010007')
p.write_text(s, encoding='utf-8')

# docs
p = root / 'README.md'
s = p.read_text(encoding='utf-8')
s = s.replace('v0.1.7', 'v0.1.8')
s = s.replace('sau mỗi 200 ms', 'sau mỗi 500 ms')
s += '\n## v0.1.8 — nhịp Confirm 500 ms\n\n- Sau callback điểm đến, controller chờ 500 ms trước lần dò/callback Xác nhận đầu tiên.\n- Các lần retry Confirm cũng cách nhau 500 ms.\n- Tối đa 40 lần, tương đương khoảng 20 giây thay vì 8 giây.\n- Bridge/game UI thread vẫn không bị Sleep; delay chỉ nằm ở controller.\n'
p.write_text(s, encoding='utf-8')

p = root / 'SEMANTIC_UI_CALLBACK_DATA.md'
s = p.read_text(encoding='utf-8')
s += '\n\n## Runtime pacing update v0.1.8\n\nChuỗi Xa Truyền dùng controller-side pacing 500 ms giữa callback destination và semantic confirm scan/callback. Không Sleep trong Bridge/game UI thread. Mục đích là tránh poll quá gấp và cho popup runtime đủ thời gian ổn định.\n'
p.write_text(s, encoding='utf-8')

import 'dart:ui';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/gestures.dart';

int encodeHotkey(Set<int> vkCodes) {
  final list = vkCodes.toList();
  int r = 0;
  for (int i = 0; i < list.length && i < 4; ++i) {
    r |= (list[i] & 0xFF) << (i * 8);
  }
  return r;
}

Set<int> decodeHotkey(int encoded) {
  final set = <int>{};
  for (int i = 0; i < 4; ++i) {
    final vk = (encoded >> (i * 8)) & 0xFF;
    if (vk != 0) set.add(vk);
  }
  return set;
}

String formatVkHotkey(Set<int> vkCodes) {
  final parts = <String>[];
  if (vkCodes.contains(0x11)) parts.add('Ctrl');
  if (vkCodes.contains(0x10)) parts.add('Shift');
  if (vkCodes.contains(0x12)) parts.add('Alt');
  if (vkCodes.contains(0x5B)) parts.add('Win');

  final others = vkCodes
      .where((v) => ![0x11, 0x10, 0x12, 0x5B].contains(v))
      .toList();
  if (others.isNotEmpty) {
    final v = others.first;
    parts.add(_vkNameMap[v] ?? 'VK_${v.toRadixString(16).toUpperCase()}');
  }
  return parts.join('+');
}

final Map<int, String> _vkNameMap = {
  0x01: 'MouseLeft',
  0x02: 'MouseRight',
  0x04: 'MouseMiddle',
  0x05: 'MouseX1',
  0x06: 'MouseX2',
  0xA0: 'LShift',
  0xA1: 'RShift',
  0xA2: 'LCtrl',
  0xA3: 'RCtrl',
  0xA4: 'LAlt',
  0xA5: 'RAlt',
  // A-Z
  for (var i = 0x41; i <= 0x5A; i++) i: String.fromCharCode(i),
  // 0-9
  for (var i = 0x30; i <= 0x39; i++) i: String.fromCharCode(i),
  0x70: 'F1', 0x71: 'F2', 0x72: 'F3', 0x73: 'F4',
  0x74: 'F5', 0x75: 'F6', 0x76: 'F7', 0x77: 'F8',
  0x78: 'F9', 0x79: 'F10', 0x7A: 'F11', 0x7B: 'F12',
};

typedef AsyncIntGetter = Future<int?> Function();
typedef AsyncIntSetter = Future<bool> Function(int newValue);

class AsyncHotkeyInput extends StatefulWidget {
  final AsyncIntGetter getter;
  final AsyncIntSetter setter;
  final String? label;

  const AsyncHotkeyInput({
    Key? key,
    required this.getter,
    required this.setter,
    this.label,
  }) : super(key: key);

  @override
  State<AsyncHotkeyInput> createState() => _AsyncHotkeyInputState();
}

class _AsyncHotkeyInputState extends State<AsyncHotkeyInput> {
  final FocusNode _focusNode = FocusNode();
  bool _loading = true, _setting = false, _editing = false;
  int _hotkeyInt = 0;
  Set<int> _pressed = {};
  String _display = '';

  @override
  void initState() {
    super.initState();
    _load();
  }

Future<void> _load() async {
  try {
    final v = await widget.getter();
    if (v != null && v != 0) {
      _hotkeyInt = v;
      _pressed = decodeHotkey(v);
      _display = formatVkHotkey(_pressed);
    }
  } catch (e, stack) {
    debugPrint('AsyncHotkeyInput.load error: $e\n$stack');
    _display = '加载失败';
  } finally {
    setState(() => _loading = false);
  }
}


  void _startEdit() {
    setState(() {
      _editing = true;
      _pressed.clear();
      _display = '请按下热键';
    });
    _focusNode.requestFocus();
  }

  Future<void> _submit() async {
    if (_pressed.isEmpty) return;
    final code = encodeHotkey(_pressed);
    setState(() => _setting = true);
    if (!await widget.setter(code)) {
      setState(() => _setting = false);
      return;
    }
    setState(() {
      _hotkeyInt = code;
      _display = formatVkHotkey(_pressed);
      _editing = false;
      _pressed.clear();
      _setting = false;
    });
    _focusNode.unfocus();
  }

  void _onKey(RawKeyEvent ev) {
    if (!_editing) return;
    if (ev is RawKeyDownEvent) {
      if (ev.data is RawKeyEventDataWindows) {
        final vk = (ev.data as RawKeyEventDataWindows).keyCode;
        setState(() {
          _pressed.add(vk);
          _display = formatVkHotkey(_pressed);
        });
      }
    }
  }

  void _onPointerDown(PointerDownEvent event) {
    if (!_editing) return;
    int? vk;

    // 鼠标按钮映射为 VK
    if (event.kind == PointerDeviceKind.mouse) {
      final buttons = event.buttons;
      if ((buttons & kPrimaryMouseButton) != 0) vk = 0x01;
      else if ((buttons & kSecondaryMouseButton) != 0) vk = 0x02;
      else if ((buttons & kMiddleMouseButton) != 0) vk = 0x04;
      else if ((buttons & kBackMouseButton) != 0) vk = 0x05;
      else if ((buttons & kForwardMouseButton) != 0) vk = 0x06;
    }

    if (vk != null) {
      setState(() {
        _pressed.add(vk!);
        _display = formatVkHotkey(_pressed);
      });
    }
  }

  @override
  Widget build(BuildContext c) {
    if (_loading) {
      return const SizedBox(width: 43, height: 43, child: CircularProgressIndicator(color: Colors.white));
    }

    return SizedBox(
      height: 43,
      width: 240,
      child: Listener(
        onPointerDown: _onPointerDown,
        behavior: HitTestBehavior.opaque,
        child: RawKeyboardListener(
          focusNode: _focusNode,
          onKey: _onKey,
          autofocus: false,
          child: Row(children: [
            if (widget.label != null)
              Padding(
                padding: const EdgeInsets.only(right: 8),
                child: Text(widget.label!, style: const TextStyle(color: Colors.white)),
              ),
            Expanded(
              child: Stack(
                alignment: Alignment.centerRight,
                children: [
                  ClipRRect(
                    borderRadius: BorderRadius.circular(10),
                    child: BackdropFilter(
                      filter: ImageFilter.blur(sigmaX: 4, sigmaY: 4),
                      child: Container(
                        height: 43,
                        padding: const EdgeInsets.symmetric(horizontal: 12),
                        alignment: Alignment.centerLeft,
                        decoration: BoxDecoration(
                          color: Colors.white.withOpacity(0.1),
                          borderRadius: BorderRadius.circular(10),
                        ),
                        child: Text(
                          _display.isEmpty ? (_editing ? '请按下热键' : '无热键') : _display,
                          style: TextStyle(
                            color: _display.isEmpty ? Colors.white54 : Colors.white,
                            fontFamily: 'ui_font',
                          ),
                        ),
                      ),
                    ),
                  ),
                  if (_setting)
                    const Positioned(
                      right: 36,
                      child: SizedBox(width: 16, height: 16, child: CircularProgressIndicator(strokeWidth: 2, color: Colors.white)),
                    ),
                  if (_editing && _pressed.isNotEmpty)
                    Positioned(
                      right: 4,
                      child: IconButton(icon: const Icon(Icons.save, color: Colors.white, size: 20), onPressed: _submit),
                    ),
                  if (!_editing && !_setting)
                    Positioned(
                      right: 4,
                      child: IconButton(icon: const Icon(Icons.edit, color: Colors.white, size: 20), onPressed: _startEdit),
                    ),
                ],
              ),
            )
          ]),
        ),
      ),
    );
  }

  @override
  void dispose() {
    _focusNode.dispose();
    super.dispose();
  }
}

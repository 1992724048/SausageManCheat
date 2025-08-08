import 'dart:ui';
import 'package:flutter/material.dart';

typedef AsyncValueGetter<T> = Future<T> Function();
typedef AsyncValueSetter<T> = Future<bool> Function(T newValue);

class AsyncNumberInput<T extends num> extends StatefulWidget {
  final AsyncValueGetter<T> getter;
  final AsyncValueSetter<T> setter;
  final String? label;

  const AsyncNumberInput({
    super.key,
    required this.getter,
    required this.setter,
    this.label,
  });

  @override
  State<AsyncNumberInput<T>> createState() => _AsyncNumberInputState<T>();
}

class _AsyncNumberInputState<T extends num> extends State<AsyncNumberInput<T>> {
  final TextEditingController _controller = TextEditingController();
  bool _loading = true;
  bool _setting = false;
  T? _value;

  // 根据 T 决定解析方式
  T? _parse(String src) {
    try {
      if (T == int) {
        return int.parse(src) as T;
      } else if (T == double) {
        return double.parse(src) as T;
      }
    } catch (_) {}
    return null;
  }

  @override
  void initState() {
    super.initState();
    _loadValue();
  }

  Future<void> _loadValue() async {
    try {
      final v = await widget.getter();
      setState(() {
        _value = v;
        _controller.text = v.toString();
        _loading = false;
      });
    } catch (e) {
      debugPrint("加载失败: $e");
      setState(() {
        _value = (T == int ? 0 : 0.0) as T;
        _controller.text = _value.toString();
        _loading = false;
      });
    }
  }

  Future<void> _submit(String input) async {
    final newValue = _parse(input);
    if (newValue == null) {
      // 解析失败，恢复原值
      _controller.text = _value?.toString() ?? '';
      return;
    }

    if (newValue == _value) return;

    setState(() => _setting = true);

    bool success = false;
    try {
      success = await widget.setter(newValue);
    } catch (e) {
      debugPrint("设置异常: $e");
      success = false;
    }

    setState(() => _setting = false);

    if (success) {
      setState(() => _value = newValue);
    } else {
      _controller.text = _value?.toString() ?? '';
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('设置失败')),
        );
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    if (_loading) {
      return const SizedBox(
        height: 43,
        width: 43,
        child: Center(child: CircularProgressIndicator(color: Colors.white)),
      );
    }

    return SizedBox(
      height: 43,
      width: 120,
      child: Row(
        children: [
          if (widget.label != null)
            Padding(
              padding: const EdgeInsets.only(right: 8.0),
              child: Text(
                widget.label!,
                style: const TextStyle(color: Colors.white),
              ),
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
                      alignment: Alignment.center,
                      padding: const EdgeInsets.symmetric(horizontal: 12),
                      decoration: BoxDecoration(
                        color: Colors.white.withOpacity(0.1),
                        borderRadius: BorderRadius.circular(10),
                      ),
                      child: IgnorePointer(
                        ignoring: _setting,
                        child: TextField(
                          controller: _controller,
                          enabled: !_setting,
                          textAlign: TextAlign.center,
                          cursorColor: Colors.white,
                          keyboardType: TextInputType.numberWithOptions(
                            decimal: T == double,
                          ),
                          onSubmitted: _submit,
                          style: const TextStyle(
                            color: Colors.white,
                            fontFamily: "ui_font",
                          ),
                          decoration: const InputDecoration(
                            isDense: true,
                            contentPadding: EdgeInsets.zero,
                            border: InputBorder.none,
                            hintStyle: TextStyle(color: Colors.white54),
                          ),
                        ),
                      ),
                    ),
                  ),
                ),
                if (_setting)
                  const Padding(
                    padding: EdgeInsets.only(right: 12),
                    child: SizedBox(
                      width: 16,
                      height: 16,
                      child: CircularProgressIndicator(
                        strokeWidth: 2,
                        color: Colors.white,
                      ),
                    ),
                  ),
              ],
            ),
          ),
        ],
      ),
    );
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }
}
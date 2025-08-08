import 'package:flutter/material.dart';

typedef AsyncBoolGetter = Future<bool> Function();
typedef AsyncBoolSetter = Future<bool> Function(bool value);
typedef ValueCallback = void Function(bool newValue, bool success);

class AsyncSwitch extends StatefulWidget {
  final AsyncBoolGetter getter;
  final AsyncBoolSetter setter;
  final String? label;
  final bool defaultValue;
  final ValueCallback? onChangedCallback;
  final bool showLoading;

  const AsyncSwitch({
    super.key,
    required this.getter,
    required this.setter,
    this.label,
    this.defaultValue = false,
    this.onChangedCallback,
    this.showLoading = true,
  });

  @override
  State<AsyncSwitch> createState() => _AsyncSwitchState();
}

class _AsyncSwitchState extends State<AsyncSwitch> {
  bool _value = false;
  bool _updating = false;

  @override
  void initState() {
    super.initState();
    _value = widget.defaultValue;
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _loadInitialValue();
    });
  }

  Future<void> _loadInitialValue() async {
    try {
      final actualValue = await widget.getter();
      if (mounted) {
        setState(() => _value = actualValue);
      }
    } catch (e) {
      debugPrint('加载失败，使用默认值: $e');
    }
  }

  Future<void> _onToggle(bool newValue) async {
    setState(() {
      _value = newValue;
      _updating = true;
    });

    await Future.delayed(const Duration(milliseconds: 50));

    bool success = false;
    try {
      success = await widget.setter(newValue);
    } catch (e) {
      debugPrint('设置异常: $e');
    }

    if (!success) {
      setState(() {
        _value = !newValue;
        _updating = false;
      });
      if (mounted) {
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('设置失败')));
      }
    } else {
      setState(() => _updating = false);
    }

    widget.onChangedCallback?.call(newValue, success);
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        if (widget.label != null) Text(widget.label!),
        Stack(
          alignment: Alignment.center,
          children: [
            IgnorePointer(
              ignoring: _updating,
              child: Opacity(
                opacity: _updating ? 0.4 : 1.0,
                child: Switch(value: _value, onChanged: _onToggle),
              ),
            ),
            if (_updating && widget.showLoading)
              const SizedBox(
                width: 24,
                height: 24,
                child: CircularProgressIndicator(
                  strokeWidth: 2,
                  color: Colors.white,
                ),
              ),
          ],
        ),
      ],
    );
  }
}

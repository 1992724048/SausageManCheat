import 'dart:ui';
import 'package:flutter/material.dart';

typedef AsyncValueGetter = Future<String> Function();
typedef AsyncValueSetter = Future<bool> Function(String newValue);
typedef AsyncOptionsGetter = Future<List<String>> Function();
typedef AsyncDropdownChangedCallback = void Function(String newValue, bool success);

class AsyncDropdown extends StatefulWidget {
  final AsyncValueGetter getter;
  final AsyncOptionsGetter optionsGetter;
  final AsyncValueSetter setter;
  final AsyncDropdownChangedCallback? onChangedCallback;
  final String? label;

  const AsyncDropdown({
    super.key,
    required this.getter,
    required this.optionsGetter,
    required this.setter,
    this.onChangedCallback,
    this.label,
  });

  @override
  State<AsyncDropdown> createState() => _AsyncDropdownState();
}

class _AsyncDropdownState extends State<AsyncDropdown> {
  List<String> _options = [];
  String? _selectedValue;
  bool _loading = true;
  bool _setting = false;

  @override
  void initState() {
    super.initState();
    _loadOptionsAndValue();
  }

  Future<void> _loadOptionsAndValue() async {
    try {
      final options = await widget.optionsGetter();
      final current = await widget.getter();
      setState(() {
        _options = options;
        _selectedValue = options.contains(current) ? current : null;
        _loading = false;
      });
    } catch (e) {
      debugPrint("加载失败: $e");
      setState(() => _loading = false);
    }
  }

  Future<void> _onChanged(String? newValue) async {
    if (newValue == null || newValue == _selectedValue) return;

    final oldValue = _selectedValue;

    setState(() {
      _selectedValue = newValue;
      _setting = true;
    });

    bool success = false;
    try {
      success = await widget.setter(newValue);
    } catch (e) {
      debugPrint("设置失败: $e");
    }

    if (!success) {
      setState(() {
        _selectedValue = oldValue;
        _setting = false;
      });
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(content: Text('设置失败')),
        );
      }
    } else {
      setState(() {
        _setting = false;
      });
    }

    widget.onChangedCallback?.call(newValue, success);
  }

  @override
  Widget build(BuildContext context) {
    if (_loading) {
      return const Center(
        child: CircularProgressIndicator(color: Colors.white),
      );
    }

    return SizedBox(
      height: 43,
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          if (widget.label != null)
            Padding(
            padding: const EdgeInsets.only(right: 8.0),
            child: Text(widget.label!, style: const TextStyle(color: Colors.white)),
          ),
        Stack(
          alignment: Alignment.center,
          children: [
            IgnorePointer(
              ignoring: _setting,
              child: Opacity(
                opacity: _setting ? 0.4 : 1.0,
                child: DropdownButton<String>(
                  value: _options.contains(_selectedValue) ? _selectedValue : null,
                  dropdownColor: Colors.transparent,
                  style: const TextStyle(color: Colors.white),
                  underline: Container(),
                  alignment: Alignment.center,
                  elevation: 0,
                  icon: const Icon(Icons.arrow_drop_down, color: Colors.white),
                  items: _options.map((String value) {
                    return DropdownMenuItem<String>(
                      value: value,
                      child: ClipRRect(
                        borderRadius: BorderRadius.circular(10),
                        child: BackdropFilter(
                          filter: ImageFilter.blur(sigmaX: 4, sigmaY: 4),
                          child: Container(
                            alignment: Alignment.center,
                            padding: const EdgeInsets.symmetric(horizontal: 16),
                            decoration: BoxDecoration(
                              color: Colors.transparent,
                              borderRadius: BorderRadius.circular(10),
                            ),
                            child: Text(
                              value,
                              style: const TextStyle(
                                fontFamily: "ui_font",
                                color: Colors.white,
                              ),
                            ),
                          ),
                        ),
                      ),
                    );
                  }).toList(),
                  onChanged: _onChanged,
                ),
              ),
            ),
            if (_setting)
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
    ),
    );
  }
}

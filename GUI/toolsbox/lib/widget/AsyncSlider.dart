import 'package:flutter/material.dart';

typedef AsyncDoubleGetter = Future<double> Function();
typedef AsyncDoubleSetter = Future<bool> Function(double value);
typedef DoubleCallback = void Function(double newValue, bool success);

class AsyncSlider extends StatefulWidget {
  final AsyncDoubleGetter getter;
  final AsyncDoubleSetter setter;

  final String? label;
  final double defaultValue;
  final ValueChanged<double>? onChanged;
  final DoubleCallback? onChangedCallback;
  final bool showLoading;
  final double min;
  final double max;
  final int divisions;
  final bool showTickLabels;
  final double? width;
  final int fixed;

  final Color? activeTrackColor;
  final Color? inactiveTrackColor;
  final Color? thumbColor;
  final TextStyle? labelTextStyle;
  final TextStyle? valueIndicatorTextStyle;
  final TextStyle? tickTextStyle;
  final Color? valueIndicatorBackgroundColor;

  const AsyncSlider({
    super.key,
    required this.getter,
    required this.setter,
    this.label,
    this.defaultValue = 0.0,
    this.onChanged,
    this.onChangedCallback,
    this.showLoading = true,
    this.min = 0.0,
    this.max = 1.0,
    this.divisions = 10,
    this.showTickLabels = false,
    this.activeTrackColor,
    this.inactiveTrackColor,
    this.thumbColor,
    this.labelTextStyle,
    this.valueIndicatorTextStyle,
    this.tickTextStyle,
    this.width,
    this.valueIndicatorBackgroundColor,
    required this.fixed,
  });

  @override
  State<AsyncSlider> createState() => _AsyncSliderState();
}

class _AsyncSliderState extends State<AsyncSlider> {
  late double _value;
  bool _updating = false;

  @override
  void initState() {
    super.initState();
    _value = widget.defaultValue;
    WidgetsBinding.instance.addPostFrameCallback((_) => _loadInitialValue());
  }

  Future<void> _loadInitialValue() async {
    try {
      final v = await widget.getter();
      if (mounted)
        setState(() => _value = _snap(v.clamp(widget.min, widget.max)));
    } catch (e) {
      debugPrint('AsyncSlider 获取失败，使用最小值: $e');
      if (mounted) setState(() => _value = widget.min);
    }
  }

  double _snap(double v) {
    final tick = (widget.max - widget.min) / widget.divisions;
    return (v / tick).round() * tick;
  }

  Future<void> _onChangeEnd(double raw) async {
    final snapped = _snap(raw);
    setState(() {
      _value = snapped;
      _updating = true;
    });

    bool success = false;
    try {
      success = await widget.setter(snapped);
    } catch (e) {
      debugPrint('AsyncSlider 设置异常: $e');
    }

    if (!success) {
      if (mounted) {
        setState(() {
          _updating = false;
          _value = widget.min;
        });
        ScaffoldMessenger.of(
          context,
        ).showSnackBar(const SnackBar(content: Text('设置失败')));
      }
    } else {
      if (mounted) setState(() => _updating = false);
    }
    widget.onChangedCallback?.call(snapped, success);
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final tick = (widget.max - widget.min) / widget.divisions;

    final defaultLabelStyle =
        widget.labelTextStyle ??
        theme.textTheme.bodyMedium?.copyWith(
          color: theme.colorScheme.onSurface,
        );
    final defaultTickStyle =
        widget.tickTextStyle ??
        theme.textTheme.bodySmall?.copyWith(
          color: theme.colorScheme.onSurfaceVariant,
        );
    final defaultIndicatorStyle =
        widget.valueIndicatorTextStyle ??
        theme.textTheme.bodySmall?.copyWith(color: theme.colorScheme.onPrimary);
    final sliderWidget = Stack(
      alignment: Alignment.center,
      children: [
        Opacity(
          opacity: _updating ? 0.4 : 1,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            mainAxisSize: MainAxisSize.min,
            children: [
              if (widget.label != null)
                Text(widget.label!, style: defaultLabelStyle),
              SliderTheme(
                data: SliderTheme.of(context).copyWith(
                  activeTrackColor: widget.activeTrackColor,
                  inactiveTrackColor: widget.inactiveTrackColor,
                  thumbColor: widget.thumbColor,
                  valueIndicatorColor: widget.valueIndicatorBackgroundColor,
                  valueIndicatorTextStyle: defaultIndicatorStyle,
                  showValueIndicator: ShowValueIndicator.always,
                ),
                child: Slider(
                  value: _value,
                  min: widget.min,
                  max: widget.max,
                  divisions: widget.divisions,
                  label: _value.toStringAsFixed(widget.fixed),
                  onChanged: _updating
                      ? null
                      : (v) {
                          setState(() => _value = v);
                          widget.onChanged?.call(v);
                        },
                  onChangeEnd: _onChangeEnd,
                ),
              ),
              if (widget.showTickLabels)
                SizedBox(
                  height: 20,
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: List.generate(
                      widget.divisions + 1,
                      (i) => Text(
                        (widget.min + tick * i).toStringAsFixed(0),
                        style: defaultTickStyle,
                      ),
                    ),
                  ),
                ),
            ],
          ),
        ),
        if (_updating && widget.showLoading)
          const SizedBox(
            width: 24,
            height: 24,
            child: CircularProgressIndicator(strokeWidth: 2),
          ),
      ],
    );

    return widget.width == null
        ? sliderWidget
        : SizedBox(width: widget.width, child: sliderWidget);
  }
}

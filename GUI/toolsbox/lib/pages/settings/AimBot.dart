import 'dart:ffi';
import 'dart:ui';
import 'package:flutter/material.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:toolsbox/module/dart2cpp.dart';
import 'package:toolsbox/widget/AsyncNumberInput.dart';
import 'package:toolsbox/widget/AsyncSlider.dart';
import 'package:toolsbox/widget/AsyncSwitch.dart';
import 'package:toolsbox/widget/CardModule.dart';
import 'package:toolsbox/widget/ExpandFadeWidget.dart';
import 'package:material_symbols_icons/symbols.dart';
import 'package:toolsbox/widget/VerticalGlassBox.dart';
import 'package:toolsbox/widget/AsyncHotkeyInput.dart';

class AimBot extends StatefulWidget {
  const AimBot({super.key});

  @override
  State<AimBot> createState() => _AimBotState();
}

class _AimBotState extends State<AimBot> with SingleTickerProviderStateMixin {
  bool f_enable = false;
  final ScrollController controller = ScrollController();

  late AnimationController animationController;
  Animation<double>? animation;

  double targetScrollOffset = 0;

  @override
  void initState() {
    super.initState();
    targetScrollOffset = 0;
    animationController = AnimationController(vsync: this);
  }

  @override
  void dispose() {
    animationController.dispose();
    controller.dispose();
    super.dispose();
  }

  void onPointerSignal(PointerSignalEvent event) {
    if (event is PointerScrollEvent) {
      targetScrollOffset = (targetScrollOffset + event.scrollDelta.dy).clamp(
        0.0,
        controller.position.maxScrollExtent,
      );

      animationController.stop();

      animation =
          Tween<double>(
              begin: controller.offset,
              end: targetScrollOffset,
            ).animate(
              CurvedAnimation(
                parent: animationController,
                curve: Curves.linearToEaseOut,
              ),
            )
            ..addListener(() {
              controller.jumpTo(animation!.value);
            });

      animationController.duration = const Duration(milliseconds: 500);
      animationController.forward(from: 0);
    }
  }

  @override
  Widget build(BuildContext context) {
    var scrollable = Listener(
      onPointerSignal: onPointerSignal,
      child: ListView(
        controller: controller,
        padding: const EdgeInsets.only(
          left: 10,
          right: 10,
          top: 10,
          bottom: 80,
        ),
        children: [
          CardModule(
            icon: Icons.warning,
            label: "注意事项",
            description: "使用自瞄功能可能会导致游戏不稳定或被封禁",
            child: [],
          ),
          CardModule(
            icon: Symbols.motion_sensor_active_sharp,
            label: "启用自瞄",
            description: "自瞄功能总开关",
            child: [
              AsyncSwitch(
                getter: () async {
                  f_enable = await configData.invoke(
                    "aim_get",
                    params: {'field_name': 'f_enable'},
                  );
                  setState(() {});
                  return f_enable;
                },
                setter: (value) async {
                  f_enable = value;
                  setState(() {});
                  return await configData.invoke(
                    "aim_set",
                    params: {'field_name': 'f_enable', 'value': value},
                  );
                },
              ),
            ],
          ),
          ExpandFadeWidget(
            isOpen: f_enable,
            child: Column(
              children: [
                CardModule(
                  icon: Symbols.speed,
                  label: "自瞄速度",
                  description: "自瞄功能瞄准速度，该值越大，瞄准速度越慢，零帧拉枪请调成1",
                  child: [
                    AsyncSlider(
                      getter: () async {
                        return await configData.invoke(
                          "aim_get",
                          params: {'field_name': 'f_speed'},
                        );
                      },
                      setter: (value) async {
                        return await configData.invoke(
                          "aim_set",
                          params: {'field_name': 'f_speed', 'value': value},
                        );
                      },
                      min: 1.00,
                      max: 10.00,
                      divisions: 900,
                      defaultValue: 1.00,
                      fixed: 2,
                      showLoading: true,
                      activeTrackColor: Colors.white.withAlpha(180),
                      inactiveTrackColor: Colors.white.withAlpha(100),
                      thumbColor: Colors.white,
                      labelTextStyle: const TextStyle(
                        color: Colors.white,
                        fontSize: 16,
                      ),
                      valueIndicatorTextStyle: const TextStyle(
                        color: Colors.white,
                        fontSize: 12,
                      ),
                      width: 400,
                    ),
                  ],
                ),
                CardModule(
                  icon: Symbols.keyboard,
                  label: "自瞄按键",
                  description: "自瞄功能的触发按键",
                  child: [
                    AsyncHotkeyInput(
                      getter: () async {
                        return await configData.invoke(
                          "aim_get",
                          params: {'field_name': 'f_hotkey'},
                        );
                      },
                      setter: (value) async {
                        return await configData.invoke(
                          "aim_set",
                          params: {'field_name': 'f_hotkey', 'value': value},
                        );
                      },
                    ),
                  ],
                ),
                VerticalGlassBox(
                  children: [
                    CardModuleTiny(
                      icon: Symbols.arrow_range,
                      label: "范围宽度",
                      description: "自瞄功能瞄准范围的宽度 (回车保存)",
                      child: [
                        AsyncNumberInput<int>(
                          getter: () async {
                            final raw = await configData.invoke( 
                              "aim_get",
                              params: {'field_name': 'f_rect_w'},
                            );
                            return (raw as num).toInt();
                          },
                          setter: (int value) async {
                            return await configData.invoke(
                              "aim_set",
                              params: {
                                'field_name': 'f_rect_w',
                                'value': value.toInt(),
                              },
                            );
                          },
                        ),
                      ],
                    ),
                    CardModuleTiny(
                      icon: Symbols.height,
                      label: "范围高度",
                      description: "自瞄功能瞄准范围的高度 (回车保存)",
                      child: [
                        AsyncNumberInput<int>(
                          getter: () async {
                            final raw = await configData.invoke(
                              "aim_get",
                              params: {'field_name': 'f_rect_h'},
                            );
                            return (raw as num).toInt();
                          },
                          setter: (int value) async {
                            return await configData.invoke(
                              "aim_set",
                              params: {
                                'field_name': 'f_rect_h',
                                'value': value.toInt(),
                              },
                            );
                          },
                        ),
                      ],
                    ),
                  ],
                ),
              ],
            ),
          ),
        ],
      ),
    );

    return Stack(
      children: [
        scrollable,
        Align(
          alignment: Alignment.bottomCenter,
          child: IgnorePointer(child: Container(height: 80)),
        ),
      ],
    );
  }
}

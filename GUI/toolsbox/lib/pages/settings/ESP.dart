import 'dart:ffi';
import 'dart:ui';
import 'package:flutter/material.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:toolsbox/module/dart2cpp.dart';
import 'package:toolsbox/widget/AsyncSwitch.dart';
import 'package:toolsbox/widget/CardModule.dart';
import 'package:toolsbox/widget/ExpandFadeWidget.dart';
import 'package:material_symbols_icons/symbols.dart';

class ESP extends StatefulWidget {
  const ESP({super.key});

  @override
  State<ESP> createState() => _ESPState();
}

class _ESPState extends State<ESP> with SingleTickerProviderStateMixin {
  bool f_enable = false;
  bool f_show_role = false;

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
            description: "使用透视功能可能会导致游戏不稳定或被封禁",
            child: [],
          ),
          CardModule(
            icon: Icons.remove_red_eye,
            label: "启用透视",
            description: "透视功能总开关",
            child: [
              AsyncSwitch(
                getter: () async {
                  f_enable = await configData.invoke(
                    "esp_get",
                    params: {'field_name': 'f_enable'},
                  );
                  setState(() {});
                  return f_enable;
                },
                setter: (value) async {
                  f_enable = value;
                  setState(() {});
                  return await configData.invoke(
                    "esp_set",
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
                ExpandFadeWidget2(
                  alwaysShow: CardModuleTiny(
                    icon: Icons.assignment_ind,
                    label: "显示玩家",
                    description: "显示玩家的透视信息",
                    child: [
                      AsyncSwitch(
                        getter: () async {
                          f_show_role = await configData.invoke(
                            "esp_get",
                            params: {'field_name': 'f_show_role'},
                          );
                          setState(() {});
                          return f_show_role;
                        },
                        setter: (value) async {
                          f_show_role = value;
                          setState(() {});
                          return await configData.invoke(
                            "esp_set",
                            params: {
                              'field_name': 'f_show_role',
                              'value': value,
                            },
                          );
                        },
                      ),
                    ],
                  ),
                  isOpen: f_show_role,
                  children: [
                    CardModuleTiny(
                      icon: Icons.fullscreen,
                      label: "显示方框",
                      description: "框选物体在屏幕上的范围",
                      child: [
                        AsyncSwitch(
                          getter: () async {
                            bool showBox = await configData.invoke(
                              "esp_get",
                              params: {'field_name': 'f_show_box'},
                            );
                            setState(() {});
                            return showBox;
                          },
                          setter: (value) async {
                            return await configData.invoke(
                              "esp_set",
                              params: {
                                'field_name': 'f_show_box',
                                'value': value,
                              },
                            );
                          },
                        ),
                      ],
                    ),
                    CardModuleTiny(
                      icon: Icons.three_p,
                      label: "显示信息",
                      description: "在角色底部显示相关信息",
                      child: [
                        AsyncSwitch(
                          getter: () async {
                            bool showBox = await configData.invoke(
                              "esp_get",
                              params: {'field_name': 'f_show_info'},
                            );
                            setState(() {});
                            return showBox;
                          },
                          setter: (value) async {
                            return await configData.invoke(
                              "esp_set",
                              params: {
                                'field_name': 'f_show_info',
                                'value': value,
                              },
                            );
                          },
                        ),
                      ],
                    ),
                    CardModuleTiny(
                      icon: Icons.accessibility_new,
                      label: "显示骨骼",
                      description: "显示玩家骨骼点",
                      child: [
                        AsyncSwitch(
                          getter: () async {
                            bool showBox = await configData.invoke(
                              "esp_get",
                              params: {'field_name': 'f_show_bone'},
                            );
                            setState(() {});
                            return showBox;
                          },
                          setter: (value) async {
                            return await configData.invoke(
                              "esp_set",
                              params: {
                                'field_name': 'f_show_bone',
                                'value': value,
                              },
                            );
                          },
                        ),
                      ],
                    ),
                    CardModuleTiny(
                      icon: Icons.sports_kabaddi,
                      label: "显示队友",
                      description: "显示自己的队友",
                      child: [
                        AsyncSwitch(
                          getter: () async {
                            bool showBox = await configData.invoke(
                              "esp_get",
                              params: {'field_name': 'f_show_team'},
                            );
                            setState(() {});
                            return showBox;
                          },
                          setter: (value) async {
                            return await configData.invoke(
                              "esp_set",
                              params: {
                                'field_name': 'f_show_team',
                                'value': value,
                              },
                            );
                          },
                        ),
                      ],
                    ),
                  ],
                ),
                CardModule(
                  icon: Icons.inventory_2,
                  label: "显示物品",
                  description: "显示周围的物品",
                  child: [
                    AsyncSwitch(
                      getter: () async {
                        bool showBox = await configData.invoke(
                          "esp_get",
                          params: {'field_name': 'f_show_item'},
                        );
                        setState(() {});
                        return showBox;
                      },
                      setter: (value) async {
                        return await configData.invoke(
                          "esp_set",
                          params: {'field_name': 'f_show_item', 'value': value},
                        );
                      },
                    ),
                  ],
                ),
                CardModule(
                  icon: Icons.car_crash,
                  label: "显示载具",
                  description: "显示附近的载具",
                  child: [
                    AsyncSwitch(
                      getter: () async {
                        bool showBox = await configData.invoke(
                          "esp_get",
                          params: {'field_name': 'f_show_cars'},
                        );
                        setState(() {});
                        return showBox;
                      },
                      setter: (value) async {
                        return await configData.invoke(
                          "esp_set",
                          params: {'field_name': 'f_show_cars', 'value': value},
                        );
                      },
                    ),
                  ],
                ),
                CardModule(
                  icon: Symbols.robot_2_sharp,
                  label: "显示人机",
                  description: "显示附近的人机",
                  child: [
                    AsyncSwitch(
                      getter: () async {
                        bool showBox = await configData.invoke(
                          "esp_get",
                          params: {'field_name': 'f_show_ai'},
                        );
                        setState(() {});
                        return showBox;
                      },
                      setter: (value) async {
                        return await configData.invoke(
                          "esp_set",
                          params: {'field_name': 'f_show_ai', 'value': value},
                        );
                      },
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

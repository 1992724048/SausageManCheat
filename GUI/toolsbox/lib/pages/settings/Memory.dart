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

class Memory extends StatefulWidget {
  const Memory({super.key});

  @override
  State<Memory> createState() => _MemoryState();
}

class _MemoryState extends State<Memory> with SingleTickerProviderStateMixin {
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

  var f_damage_multi = false;

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
            description: "使用内存功能可能会导致游戏不稳定或被封禁",
            child: [],
          ),
          // CardModule(
          //   icon: Symbols.eye_tracking,
          //   label: "子弹追踪",
          //   description: "子弹直接生成在敌人头部，如果子弹初速过快可能会被游戏引擎伪物理效果判断为没有碰撞",
          //   child: [
          //     AsyncSwitch(
          //       getter: () async {
          //         f_enable = await configData.invoke(
          //           "mem_get",
          //           params: {'field_name': 'f_bullet_tracking'},
          //         );
          //         setState(() {});
          //         return f_enable;
          //       },
          //       setter: (value) async {
          //         f_enable = value;
          //         setState(() {});
          //         return await configData.invoke(
          //           "mem_set",
          //           params: {'field_name': 'f_bullet_tracking', 'value': value},
          //         );
          //       },
          //     ),
          //   ],
          // ),
          CardModule(
            icon: Symbols.recenter,
            label: "弹道追踪",
            description: "子弹生成后朝目标方向飞去 (和子追不兼容)",
            child: [
              AsyncSwitch(
                getter: () async {
                  f_enable = await configData.invoke(
                    "mem_get",
                    params: {'field_name': 'f_ballistics_tracking'},
                  );
                  setState(() {});
                  return f_enable;
                },
                setter: (value) async {
                  f_enable = value;
                  setState(() {});
                  return await configData.invoke(
                    "mem_set",
                    params: {
                      'field_name': 'f_ballistics_tracking',
                      'value': value,
                    },
                  );
                },
              ),
            ],
          ),
          CardModule(
            icon: Symbols.detection_and_zone,
            label: "自瞄禁锢",
            description: "瞄准敌人时锁定敌人位置",
            child: [
              AsyncSwitch(
                getter: () async {
                  f_enable = await configData.invoke(
                    "mem_get",
                    params: {'field_name': 'f_lock_role'},
                  );
                  setState(() {});
                  return f_enable;
                },
                setter: (value) async {
                  f_enable = value;
                  setState(() {});
                  return await configData.invoke(
                    "mem_set",
                    params: {'field_name': 'f_lock_role', 'value': value},
                  );
                },
              ),
            ],
          ),
          // CardModule(
          //   icon: Symbols.face_5,
          //   label: "枪枪打头",
          //   description: "不管击中哪个部位都判定为头部",
          //   child: [
          //     AsyncSwitch(
          //       getter: () async {
          //         f_enable = await configData.invoke(
          //           "mem_get",
          //           params: {'field_name': 'f_all_hit_head'},
          //         );
          //         setState(() {});
          //         return f_enable;
          //       },
          //       setter: (value) async {
          //         f_enable = value;
          //         setState(() {});
          //         return await configData.invoke(
          //           "mem_set",
          //           params: {'field_name': 'f_all_hit_head', 'value': value},
          //         );
          //       },
          //     ),
          //   ],
          // ),
          // CardModule(
          //   icon: Symbols.autoplay,
          //   label: "全枪自动",
          //   description: "所有枪械都拥有全自动模式",
          //   child: [
          //     AsyncSwitch(
          //       getter: () async {
          //         f_enable = await configData.invoke(
          //           "mem_get",
          //           params: {'field_name': 'f_all_gun_auto'},
          //         );
          //         setState(() {});
          //         return f_enable;
          //       },
          //       setter: (value) async {
          //         f_enable = value;
          //         setState(() {});
          //         return await configData.invoke(
          //           "mem_set",
          //           params: {'field_name': 'f_all_gun_auto', 'value': value},
          //         );
          //       },
          //     ),
          //   ],
          // ),
          // ExpandFadeWidget2(
          //   alwaysShow: CardModuleTiny(
          //     icon: Symbols.azm,
          //     label: "伤害翻倍",
          //     description: "伤害翻到指定倍数，过高会导致对面雪女复活状态直接结束",
          //     child: [
          //       AsyncSwitch(
          //         getter: () async {
          //           f_damage_multi = await configData.invoke(
          //             "mem_get",
          //             params: {'field_name': 'f_damage_multi'},
          //           );
          //           setState(() {});
          //           return f_enable;
          //         },
          //         setter: (value) async {
          //           f_damage_multi = value;
          //           setState(() {});
          //           return await configData.invoke(
          //             "mem_set",
          //             params: {'field_name': 'f_damage_multi', 'value': value},
          //           );
          //         },
          //       ),
          //     ],
          //   ),
          //   isOpen: f_damage_multi,
          //   children: [
          //     CardModuleTiny(
          //       icon: Symbols.speed_2x,
          //       label: "伤害倍数",
          //       description: "换算：基础伤害 + 基础伤害 * 倍数",
          //       child: [
          //         AsyncSlider(
          //           getter: () async {
          //             return await configData.invoke(
          //               "mem_get",
          //               params: {'field_name': 'f_damage_multi_value'},
          //             );
          //           },
          //           setter: (value) async {
          //             return await configData.invoke(
          //               "mem_set",
          //               params: {
          //                 'field_name': 'f_damage_multi_value',
          //                 'value': value.toInt(),
          //               },
          //             );
          //           },
          //           min: 1,
          //           max: 20,
          //           divisions: 19,
          //           defaultValue: 1,
          //           fixed: 0,
          //           showLoading: true,
          //           activeTrackColor: Colors.white.withAlpha(180),
          //           inactiveTrackColor: Colors.white.withAlpha(100),
          //           thumbColor: Colors.white,
          //           labelTextStyle: const TextStyle(
          //             color: Colors.white,
          //             fontSize: 16,
          //           ),
          //           valueIndicatorTextStyle: const TextStyle(
          //             color: Colors.white,
          //             fontSize: 12,
          //           ),
          //           width: 400,
          //         ),
          //       ],
          //     ),
          //   ],
          // ),
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

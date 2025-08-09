import 'dart:ffi';
import 'dart:ui';
import 'package:flutter/material.dart';
import 'package:flutter/gestures.dart';
import 'package:flutter/services.dart';
import 'package:toolsbox/widget/AsyncDropdown.dart';
import 'package:toolsbox/widget/AsyncSwitch.dart';
import 'package:toolsbox/widget/CardModule.dart';
import 'package:toolsbox/module/dart2cpp.dart';
import 'package:toolsbox/widget/ExpandFadeWidget.dart';
import 'package:material_symbols_icons/symbols.dart';

class Launcher extends StatefulWidget {
  const Launcher({super.key});

  @override
  State<Launcher> createState() => _LauncherState();
}

class _LauncherState extends State<Launcher>
    with SingleTickerProviderStateMixin {
  bool no_sycl = true;
  final List<String> download_options = ['Github'];
  String download_selectedValue = 'Github';
  Key _deviceDropdownKey = UniqueKey();

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
    Widget scrollable = ListView(
      controller: controller,
      padding: const EdgeInsets.only(left: 10, right: 10, top: 10, bottom: 80),
      children: [
        CardModule(
          icon: Icons.update,
          label: '检查更新',
          description: '启动时自动检测辅助更新',
          child: [
            AsyncSwitch(
              getter: () async {
                return await configData.invoke(
                  "config_get",
                  params: {'field_name': 'f_auto_chek_update'},
                );
              },
              setter: (value) async {
                return await configData.invoke(
                  "config_set",
                  params: {'field_name': 'f_auto_chek_update', 'value': value},
                );
              },
            ),
          ],
        ),
        CardModule(
          icon: Icons.flaky,
          label: '文件校验',
          description: '启动时检测文件资源完整性',
          child: [
            AsyncSwitch(
              getter: () async {
                return await configData.invoke(
                  "config_get",
                  params: {'field_name': 'f_check_file'},
                );
              },
              setter: (value) async {
                return await configData.invoke(
                  "config_set",
                  params: {'field_name': 'f_check_file', 'value': value},
                );
              },
            ),
          ],
        ),
        CardModule(
          icon: Icons.account_tree,
          label: '多线程下载',
          description: '下载文件时使用多线程并发',
          child: [
            AsyncSwitch(
              getter: () async {
                return await configData.invoke(
                  "config_get",
                  params: {'field_name': 'f_multi_thread'},
                );
              },
              setter: (value) async {
                return await configData.invoke(
                  "config_set",
                  params: {'field_name': 'f_multi_thread', 'value': value},
                );
              },
            ),
          ],
        ),
        CardModule(
          icon: Icons.storage,
          label: '更新下载源',
          description: '选择更新与下载的服务器来源 (等待第三方源提供支持)',
          child: [
            Center(
              child: DropdownButton<String>(
                value: download_selectedValue,
                dropdownColor: Colors.transparent,
                style: const TextStyle(color: Colors.white),
                menuMaxHeight: 200,
                underline: Container(),
                alignment: Alignment.center,
                elevation: 0,
                icon: const Icon(Icons.arrow_drop_down, color: Colors.white),
                items: download_options.map<DropdownMenuItem<String>>((
                  String value,
                ) {
                  return DropdownMenuItem<String>(
                    value: value,
                    child: ClipRRect(
                      borderRadius: BorderRadius.circular(10),
                      child: BackdropFilter(
                        filter: ImageFilter.blur(sigmaX: 4, sigmaY: 4),
                        child: Container(
                          alignment: Alignment.center,
                          padding: const EdgeInsets.symmetric(
                            horizontal: 16,
                            vertical: 0,
                          ),
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
                onChanged: download_onSelect,
              ),
            ),
          ],
        ),
        ExpandFadeWidget2(
          alwaysShow: CardModuleTiny(
            icon: Icons.developer_board_off,
            label: '不使用SYCL',
            description: '将使用默认的TBB在CPU上运行',
            child: [
              AsyncSwitch(
                getter: () async {
                  no_sycl = await configData.invoke(
                    "config_get",
                    params: {'field_name': 'f_no_sycl'},
                  );
                  setState(() {});
                  return no_sycl;
                },
                setter: (value) async {
                  var done = await configData.invoke(
                    "config_set",
                    params: {'field_name': 'f_no_sycl', 'value': value},
                  );
                  if (done) {
                    no_sycl = value;
                    setState(() {});
                  }
                  return done;
                },
              ),
            ],
          ),
          isOpen: !no_sycl,
          children: [
            CardModuleTiny(
              icon: Icons.memory,
              label: '计算单元',
              description:
                  '优先级 GPU(独显)>GPU(核显)>CPU (需要硬件支持，没有可用设备将默认使用TBB在CPU上运行)',
              child: [
                Center(
                  child: AsyncDropdown(
                    getter: () async {
                      return await configData.invoke(
                        "config_get",
                        params: {'field_name': 'f_calc_xpu'},
                      );
                    },
                    optionsGetter: () async {
                      final dynamic result = await configData.invoke(
                        "config_get",
                        params: {'field_name': 'f_calc_xpu_options'},
                      );
                      if (result is List) {
                        return result.cast<String>();
                      }
                      return [];
                    },
                    setter: (value) async {
                      final result = await configData.invoke(
                        "config_set",
                        params: {'field_name': 'f_calc_xpu', 'value': value},
                      );
                      setState(() {
                        _deviceDropdownKey = UniqueKey();
                      });
                      return result;
                    },
                  ),
                ),
              ],
            ),
            CardModuleTiny(
              icon: Icons.schema,
              label: '计算设备',
              description: '选择用于计算的设备 (选择GPU(独显)或GPU(核显)可以缓解CPU负载)',
              child: [
                Center(
                  child: AsyncDropdown(
                    key: _deviceDropdownKey,
                    getter: () async {
                      return await configData.invoke(
                        "config_get",
                        params: {'field_name': 'f_calc_process'},
                      );
                    },
                    optionsGetter: () async {
                      final dynamic result = await configData.invoke(
                        "config_get",
                        params: {'field_name': 'f_calc_process_options'},
                      );
                      if (result is List) {
                        return result.cast<String>();
                      }
                      return [];
                    },
                    setter: (value) async {
                      return await configData.invoke(
                        "config_set",
                        params: {
                          'field_name': 'f_calc_process',
                          'value': value,
                        },
                      );
                    },
                  ),
                ),
              ],
            ),
          ],
        ),
      ],
    );

    scrollable = Listener(onPointerSignal: onPointerSignal, child: scrollable);

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

  void download_onSelect(String? value) {
    setState(() {
      download_selectedValue = value!;
    });
  }
}

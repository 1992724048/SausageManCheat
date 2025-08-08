import 'dart:async';
import 'dart:io';
import 'dart:ui';
import 'package:flutter/material.dart';
import 'package:toolsbox/module/dart2cpp.dart';
import 'package:toolsbox/pages/about.dart';
import 'package:toolsbox/pages/setting.dart';
import 'package:window_manager/window_manager.dart';
import 'package:bitsdojo_window/bitsdojo_window.dart';
import 'pages/home.dart';

double radius = 50;
double blur = 8;

Future<void> main() async {
  runApp(const ToolsBoxUI());

  doWhenWindowReady(() {
    final win = appWindow;
    const initialSize = Size(1280, 720);
    win.minSize = initialSize;
    win.maxSize = initialSize;
    win.size = initialSize;
    win.alignment = Alignment.center;
    win.show();
  });
}

class ToolsBoxUI extends StatelessWidget {
  const ToolsBoxUI({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'ToolsBoxUI',
      theme: ThemeData(
        useMaterial3: true,
        useSystemColors: true,
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.blue),
        fontFamily: "ui_font",
        switchTheme: SwitchThemeData(
          trackColor: MaterialStateProperty.all<Color>(
            Colors.white.withOpacity(0.2),
          ),
          thumbColor: MaterialStateProperty.all<Color>(
            Colors.white.withOpacity(0.3),
          ),
          trackOutlineColor: MaterialStateProperty.all<Color>(
            Colors.blueAccent.withOpacity(0),
          ),
        ),
        dropdownMenuTheme: DropdownMenuThemeData(
          textStyle: TextStyle(color: Colors.white),
          inputDecorationTheme: InputDecorationTheme(
            fillColor: Colors.white,
            suffixIconColor: Colors.white,
            prefixIconColor: Colors.white
          ),
        ),
      ),
      home: Stack(
        children: [
          Positioned.fill(
            child: Image.asset('lib/images/bg.png', fit: BoxFit.cover),
          ),
          Positioned(
            left: 0,
            right: 0,
            top: 0,
            height: 40,
            child: const TitleUI(),
          ),
          Positioned.fill(
            child: Padding(
              padding: const EdgeInsets.only(top: 40),
              child: Row(children: [Expanded(child: MainUI())]),
            ),
          ),
        ],
      ),
    );
  }
}

class TitleUI extends StatefulWidget {
  const TitleUI({super.key});

  @override
  State<TitleUI> createState() => _TitleUIState();
}

class _TitleUIState extends State<TitleUI> {
  bool isAlwaysOnTop = false;

  @override
  void initState() {
    super.initState();
    _initAlwaysOnTop();
  }

  Future<void> _initAlwaysOnTop() async {
    final value = await windowManager.isAlwaysOnTop();
    setState(() {
      isAlwaysOnTop = value;
    });
  }

  @override
  Widget build(BuildContext context) {
    return WindowTitleBarBox(
      child: ClipRect(
        child: BackdropFilter(
          filter: ImageFilter.blur(sigmaX: 10, sigmaY: 10),
          child: Container(
            height: 40,
            width: double.infinity,
            decoration: BoxDecoration(
              color: Colors.black.withOpacity(0.10),
              borderRadius: BorderRadius.zero,
              border: Border.all(
                color: Colors.blue.withOpacity(0.10),
                width: 1.0,
              ),
            ),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                Expanded(
                  child: MoveWindow(
                    child: Row(
                      children: [
                        Padding(
                          padding: const EdgeInsets.only(right: 10),
                          child: null,
                        ),
                      ],
                    ),
                  ),
                ),

                _TitleBarButton(
                  icon: Icons.remove,
                  tooltip: '最小化',
                  onPressed: () {
                    appWindow.minimize();
                  },
                ),
                _TitleBarButton(
                  icon: Icons.close_rounded,
                  tooltip: '关闭',
                  onPressed: () {
                    appWindow.hide();
                  },
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

class _TitleBarButton extends StatelessWidget {
  final IconData icon;
  final String tooltip;
  final VoidCallback onPressed;

  const _TitleBarButton({
    required this.icon,
    required this.tooltip,
    required this.onPressed,
  });

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 36,
      height: 36,
      child: IconButton(
        icon: Icon(icon, color: Colors.white),
        iconSize: 24,
        padding: EdgeInsets.zero,
        constraints: const BoxConstraints(
          minWidth: 36,
          minHeight: 36,
          maxHeight: 36,
          maxWidth: 36,
        ),
        tooltip: tooltip,
        splashRadius: 18,
        onPressed: onPressed,
        style: ButtonStyle(
          shape: WidgetStateProperty.all(
            RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
          ),
        ),
      ),
    );
  }
}

class MainUI extends StatefulWidget {
  const MainUI({super.key});

  @override
  State<MainUI> createState() => _MainUIState();
}

class _MainUIState extends State<MainUI>
    with SingleTickerProviderStateMixin, AutomaticKeepAliveClientMixin<MainUI> {
  int _selectedIndex = 0;

  final List<Widget> _pages = [HomePage(), Setting(), AboutPage()];

  void _onSelect(int index) {
    setState(() {
      _selectedIndex = index;
    });
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Stack(
      children: [
        AnimatedSwitcher(
          duration: const Duration(milliseconds: 500),
          transitionBuilder: (Widget child, Animation<double> animation) {
            final isForward =
                _selectedIndex >=
                (!_pages.contains(child)
                    ? _selectedIndex
                    : _pages.indexOf(child));
            return SlideTransition(
              position:
                  Tween<Offset>(
                    begin: Offset(isForward ? 1.0 : -1.0, 0.0),
                    end: Offset.zero,
                  ).animate(
                    CurvedAnimation(
                      parent: animation,
                      curve: Curves.fastEaseInToSlowEaseOut,
                    ),
                  ),
              child: child,
            );
          },
          child: Container(
            key: ValueKey<int>(_selectedIndex),
            width: double.infinity,
            height: double.infinity,
            alignment: Alignment.topLeft,
            color: Colors.transparent,
            child: _pages[_selectedIndex],
          ),
        ),
        Positioned(
          right: 20,
          bottom: 20,
          child: Row(
            children: [
              SelectBar(selectedIndex: _selectedIndex, onSelect: _onSelect),
              const SizedBox(width: 12),
              InjectBar(
                onInject: () {
                  setState(() {});
                },
              ),
            ],
          ),
        ),
      ],
    );
  }

  @override
  bool get wantKeepAlive => true;
}

class SelectBar extends StatelessWidget {
  final int selectedIndex;
  final ValueChanged<int> onSelect;
  const SelectBar({
    super.key,
    required this.selectedIndex,
    required this.onSelect,
  });

  @override
  Widget build(BuildContext context) {
    return ClipRRect(
      borderRadius: BorderRadius.circular(radius),
      child: BackdropFilter(
        filter: ImageFilter.blur(sigmaX: blur, sigmaY: blur),
        child: Container(
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 8),
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(radius),
            boxShadow: [
              BoxShadow(
                color: Colors.white.withOpacity(0.35),
                blurRadius: 24,
                spreadRadius: 2,
                offset: Offset(-8, -8),
              ),
            ],

          ),
          child: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              _buildIconButton(icon: Icons.home, label: '主页', index: 0),
              _buildIconButton(icon: Icons.settings, label: '设置', index: 1),
              _buildIconButton(icon: Icons.info, label: '关于', index: 2),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildIconButton({
    required IconData icon,
    required String label,
    required int index,
  }) {
    final bool selected = selectedIndex == index;
    return IconButton(
      icon: Icon(icon, color: Colors.white),
      onPressed: () => onSelect(index),
      tooltip: label,
      style: selected
          ? ButtonStyle(
              backgroundColor: WidgetStateProperty.all(
                Colors.blue.withOpacity(0.65),
              ),
              shape: WidgetStateProperty.all(
                RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(radius),
                ),
              ),
            )
          : null,
    );
  }
}

class InjectBar extends StatefulWidget {
  final VoidCallback? onInject;
  const InjectBar({super.key, this.onInject});

  @override
  State<InjectBar> createState() => _InjectBarState();
}

class _InjectBarState extends State<InjectBar> {
  bool is_inject = false;
  Duration elapsed = Duration.zero;
  Timer? _timer;

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  void _startTimer() {
    _timer = Timer.periodic(Duration(seconds: 1), (timer) {
      setState(() {
        elapsed += Duration(seconds: 1);
      });
    });
  }

  void _stopTimer() {
    _timer?.cancel();
    _timer = null;
    setState(() {
      elapsed = Duration.zero;
    });
  }

  String _formatDuration(Duration d) {
    String twoDigits(int n) => n.toString().padLeft(2, '0');
    final hours = twoDigits(d.inHours);
    final minutes = twoDigits(d.inMinutes.remainder(60));
    final seconds = twoDigits(d.inSeconds.remainder(60));
    return '$hours:$minutes:$seconds';
  }

  Future<void> _handleInjectionToggle() async {
    try {
      if (!is_inject) {
        final result = await configData.invoke(
          "invoke",
          params: {'func_name': 'game_is_run'},
        );

        if (result == true) {
          await configData.invoke(
            "invoke",
            params: {'func_name': 'inject'},
          );
          _startTimer();
          setState(() {
            is_inject = true;
          });
        } else {
      
        }
      } else {
        await configData.invoke(
          "invoke",
          params: {'func_name': 'close'},
        );
        _stopTimer();
        setState(() {
          is_inject = false;
        });
      }

      widget.onInject?.call();
    } catch (e) {

    }
  }

  @override
  Widget build(BuildContext context) {
    return ClipRRect(
      borderRadius: BorderRadius.circular(32),
      child: BackdropFilter(
        filter: ImageFilter.blur(sigmaX: 8, sigmaY: 8),
        child: Container(
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(32),
            gradient: LinearGradient(
              begin: Alignment.topLeft,
              end: Alignment.bottomRight,
              colors: [
                Colors.blue.withOpacity(0.28),
                Colors.white.withOpacity(0.10),
              ],
              stops: [0.0, 1.0],
            ),
            boxShadow: [
              BoxShadow(
                color: is_inject
                    ? Colors.grey.withOpacity(0.35)
                    : Color.fromARGB(255, 48, 137, 200),
                blurRadius: 16,
                offset: Offset(0, 4),
              ),
            ],
          ),
          child: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              AnimatedSwitcher(
                duration: Duration(milliseconds: 100),
                transitionBuilder: (child, animation) => FadeTransition(
                  opacity: animation,
                  child: SizeTransition(
                    sizeFactor: animation,
                    axis: Axis.horizontal,
                    child: child,
                  ),
                ),
                child: is_inject
                    ? FloatingActionButton.extended(
                        key: ValueKey('extended'),
                        onPressed: _handleInjectionToggle,
                        backgroundColor: Colors.transparent,
                        elevation: 0,
                        foregroundColor: Colors.white,
                        shape: const StadiumBorder(),
                        icon: Icon(Icons.vaccines),
                        label: SizedBox(
                          width: 65,
                          child: Align(
                            alignment: Alignment.centerRight,
                            child: Text(_formatDuration(elapsed)),
                          ),
                        ),
                        tooltip: '已注入',
                      )
                    : FloatingActionButton(
                        key: ValueKey('fab'),
                        onPressed: _handleInjectionToggle,
                        backgroundColor: Colors.transparent,
                        elevation: 0,
                        foregroundColor: Colors.white,
                        shape: const CircleBorder(),
                        tooltip: '注入',
                        child: Icon(Icons.vaccines),
                      ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
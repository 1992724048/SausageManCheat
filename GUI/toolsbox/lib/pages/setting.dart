import 'dart:ui';
import 'package:flutter/material.dart';
import 'package:flutter/gestures.dart';
import 'package:toolsbox/pages/settings/AimBot.dart';
import 'package:toolsbox/pages/settings/ESP.dart';
import 'package:toolsbox/pages/settings/Memory.dart';
import 'package:toolsbox/pages/settings/launcher.dart';

double radius = 50;
double blur = 8;

class Setting extends StatefulWidget {
  const Setting({super.key});

  @override
  State<Setting> createState() => _SettingState();
}

class _SettingState extends State<Setting>
    with AutomaticKeepAliveClientMixin<Setting> {
  int _currentPageIndex = 0;
  int _targetPageIndex = 0;
  final PageController _pageController = PageController();

  @override
  bool get wantKeepAlive => true;

  final List<Widget> _pages = [
    Center(child: Launcher()),
    Center(child: ESP()),
    Center(child: AimBot()),
    Center(child: Memory()),
  ];

  void _onSelect(int index) {
    if (_targetPageIndex == index) return;

    setState(() {
      _targetPageIndex = index;
    });
    _pageController
        .animateToPage(
          index,
          duration: const Duration(milliseconds: 500),
          curve: Curves.fastEaseInToSlowEaseOut,
        )
        .then((_) {
          setState(() {
            _currentPageIndex = index;
          });
        });
  }

  @override
  void initState() {
    super.initState();
    _pageController.addListener(() {
      if (!_pageController.position.isScrollingNotifier.value) {
        final currentPage = _pageController.page?.round() ?? 0;
        if (currentPage != _currentPageIndex) {
          setState(() {
            _currentPageIndex = currentPage;
            _targetPageIndex = currentPage;
          });
        }
      }
    });
  }

  @override
  void dispose() {
    _pageController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Stack(
      children: [
        Listener(
          child: PageView(
            controller: _pageController,
            scrollDirection: Axis.horizontal,
            physics: const PageScrollPhysics(),
            children: _pages,
            onPageChanged: (index) {
              if (index != _currentPageIndex) {
                setState(() {
                  _targetPageIndex = _currentPageIndex = index;
                });
              }
            },
          ),
        ),
        Positioned(
          right: 0,
          left: 0,
          bottom: 20,
          child: Center(
            child: SelectBar(
              selectedIndex: _targetPageIndex,
              onSelect: _onSelect,
            ),
          ),
        ),
      ],
    );
  }
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
          padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.45),
            borderRadius: BorderRadius.circular(radius),
            boxShadow: [
              BoxShadow(
                color: Colors.white.withOpacity(0.25),
                blurRadius: 24,
                spreadRadius: 2,
                offset: Offset(-8, -8),
              ),
            ],
            gradient: LinearGradient(
              begin: Alignment.topLeft,
              end: Alignment.bottomRight,
              colors: [
                Colors.white.withOpacity(0.10),
                Colors.blue.withOpacity(0.10),
              ],
              stops: [0.0, 1.0],
            ),
          ),
          child: Row(
            mainAxisSize: MainAxisSize.min,
            children: [
              _buildIconButton(icon: Icons.dashboard, label: '其他', index: 0),
              _buildIconButton(icon: Icons.visibility, label: '透视', index: 1),
              _buildIconButton(icon: Icons.my_location, label: '自瞄', index: 2),
              _buildIconButton(icon: Icons.memory, label: '内存', index: 3),
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
    return Padding(
      padding: const EdgeInsets.symmetric(vertical: 4),
      child: IconButton(
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
      ),
    );
  }
}

import 'dart:ui';

import 'package:flutter/material.dart';

class ExpandFadeWidget extends StatefulWidget {
  final bool isOpen;
  final Widget child;
  final Duration duration;
  final Curve curve;

  const ExpandFadeWidget({
    super.key,
    required this.isOpen,
    required this.child,
    this.duration = const Duration(milliseconds: 200),
    this.curve = Curves.easeInOutQuart,
  });

  @override
  State<ExpandFadeWidget> createState() => _ExpandFadeWidgetState();
}

class _ExpandFadeWidgetState extends State<ExpandFadeWidget>
    with SingleTickerProviderStateMixin {
  late bool _visible;

  @override
  void initState() {
    super.initState();
    _visible = widget.isOpen;
  }

  @override
  void didUpdateWidget(covariant ExpandFadeWidget oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (widget.isOpen && !_visible) {
      setState(() {
        _visible = true;
      });
    } else if (!widget.isOpen && _visible) {
      Future.delayed(widget.duration, () {
        if (!mounted) return;
        if (!widget.isOpen) {
          setState(() {
            _visible = false;
          });
        }
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedSize(
      duration: widget.duration,
      curve: widget.curve,
      child: _visible
          ? AnimatedOpacity(
              opacity: widget.isOpen ? 1 : 0,
              duration: widget.duration,
              child: widget.child,
            )
          : const SizedBox.shrink(),
    );
  }
}

class ExpandFadeWidget2 extends StatefulWidget {
  final bool isOpen;
  final Widget alwaysShow;
  final List<Widget> children;

  const ExpandFadeWidget2({
    super.key,
    required this.isOpen,
    required this.alwaysShow,
    required this.children,
  });

  @override
  State<ExpandFadeWidget2> createState() => _ExpandFadeWidget2State();
}

class _ExpandFadeWidget2State extends State<ExpandFadeWidget2>
    with SingleTickerProviderStateMixin {
  late final AnimationController _controller;
  late final Animation<double> _animation;
  late final Animation<Offset> _slideAnimation;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      duration: const Duration(milliseconds: 250),
      vsync: this,
    );
    final curvedAnimation = CurvedAnimation(
      parent: _controller,
      curve: Curves.easeInOutQuart,
    );

    _animation = curvedAnimation;

    _slideAnimation = Tween<Offset>(
      begin: const Offset(0, -0.05),
      end: Offset.zero,
    ).animate(curvedAnimation);

    if (widget.isOpen) {
      _controller.value = 1.0;
    }
  }

  @override
  void didUpdateWidget(covariant ExpandFadeWidget2 oldWidget) {
    super.didUpdateWidget(oldWidget);
    if (oldWidget.isOpen != widget.isOpen) {
      if (widget.isOpen) {
        _controller.forward();
      } else {
        _controller.reverse();
      }
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  Widget _buildCardWrapper(Widget child) {
    return Card(
      color: Colors.transparent,
      elevation: 0,
      child: ClipRRect(
        borderRadius: BorderRadius.circular(5),
        child: BackdropFilter(
          filter: ImageFilter.blur(sigmaX: 4, sigmaY: 4),
          child: Container(
            decoration: BoxDecoration(
              borderRadius: BorderRadius.circular(5),
              boxShadow: [
                BoxShadow(
                  color: Colors.white.withOpacity(0.10),
                  blurRadius: 16,
                  offset: Offset(0, 4),
                ),
              ],
            ),
            child: Padding(
              padding: const EdgeInsets.symmetric(horizontal: 5, vertical: 8),
              child: child,
            ),
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return _buildCardWrapper(
      Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          widget.alwaysShow,
          SizeTransition(
            sizeFactor: _animation,
            axisAlignment: -1.0,
            child: FadeTransition(
              opacity: _animation,
              child: SlideTransition(
                position: _slideAnimation,
                child: Column(
                  children: [
                    Divider(color: Colors.white, thickness: 1),
                    for (int i = 0; i < widget.children.length; i++) ...[
                      widget.children[i],
                      if (i != widget.children.length - 1)
                        const Divider(
                          color: Colors.white12,
                          thickness: 1,
                          height: 20,
                        ),
                    ],
                  ],
                ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class ExpandFadeContainer extends StatefulWidget {
  final String? title;
  final List<Widget> children;
  final bool initiallyOpen;
  final VoidCallback? onMore;

  const ExpandFadeContainer({
    super.key,
    required this.title,
    required this.children,
    this.initiallyOpen = false,
    this.onMore,
  });

  @override
  State<ExpandFadeContainer> createState() => _ExpandFadeContainerState();
}

class _ExpandFadeContainerState extends State<ExpandFadeContainer> {
  late bool _isOpen;

  @override
  void initState() {
    super.initState();
    _isOpen = widget.initiallyOpen;
  }

  void _toggle() => setState(() => _isOpen = !_isOpen);

  @override
  Widget build(BuildContext context) {
    return _buildGlassCard(
      child: ExpandFadeWidget2(
        isOpen: _isOpen,
        alwaysShow: _header ?? const SizedBox.shrink(),
        children: widget.children,
      ),
    );
  }

  Widget? get _header {
    final title = widget.title;
    if (title == null || title.isEmpty) return null;

    return InkWell(
      onTap: _toggle,
      borderRadius: const BorderRadius.vertical(top: Radius.circular(5)),
      child: Padding(
        padding: const EdgeInsets.fromLTRB(12, 10, 8, 10),
        child: Row(
          children: [
            Expanded(
              child: Text(
                title,
                style: const TextStyle(
                  color: Colors.white,
                  fontSize: 16,
                  fontWeight: FontWeight.w600,
                ),
              ),
            ),
            if (widget.onMore != null)
              IconButton(
                icon: const Icon(Icons.more_horiz, color: Colors.white54),
                padding: EdgeInsets.zero,
                constraints: const BoxConstraints(),
                onPressed: widget.onMore,
              ),
            AnimatedRotation(
              turns: _isOpen ? 0.5 : 0,
              duration: const Duration(milliseconds: 250),
              child: const Icon(Icons.expand_more, color: Colors.white70),
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildGlassCard({required Widget child}) {
    return Card(
      color: Colors.transparent,
      elevation: 0,
      margin: EdgeInsets.zero,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(8)),
      child: child,
    );
  }
}

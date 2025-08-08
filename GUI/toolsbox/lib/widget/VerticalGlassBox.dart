import 'dart:ui';
import 'package:flutter/material.dart';

class VerticalGlassBox extends StatelessWidget {
  final List<Widget> children;

  const VerticalGlassBox({super.key, required this.children});

  @override
  Widget build(BuildContext context) {
    return _buildGlassCard(
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 5, vertical: 8),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisSize: MainAxisSize.min,
          children: [
            for (int i = 0; i < children.length; i++) ...[
              children[i],
              if (i != children.length - 1)
                const Divider(
                  color: Colors.white12,
                  thickness: 1,
                  height: 20,
                ),
            ],
          ],
        ),
      ),
    );
  }

  Widget _buildGlassCard({required Widget child}) {
    return Card(
      color: Colors.transparent,
      elevation: 0,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(5)),
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
                  offset: const Offset(0, 4),
                ),
              ],
            ),
            child: child,
          ),
        ),
      ),
    );
  }
}
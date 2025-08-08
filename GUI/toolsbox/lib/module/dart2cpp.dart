import 'dart:async';
import 'dart:io';
import 'dart:ui';

import 'package:flutter/foundation.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:path/path.dart' as path;
import 'package:window_manager/src/resize_edge.dart';
import 'package:window_manager/src/title_bar_style.dart';
import 'package:window_manager/src/utils/calc_window_position.dart';
import 'package:window_manager/src/window_listener.dart';
import 'package:window_manager/src/window_options.dart';

class ConfigData {
  final MethodChannel _channel = const MethodChannel('dart2cpp');
  final ObserverList<WindowListener> _listeners =
      ObserverList<WindowListener>();

  ConfigData._() {
    _channel.setMethodCallHandler(_methodCallHandler);
  }

  List<WindowListener> get listeners {
    final List<WindowListener> localListeners = List<WindowListener>.from(
      _listeners,
    );
    return localListeners;
  }

  Future<void> _methodCallHandler(MethodCall call) async {
    for (final WindowListener listener in listeners) {
      if (!_listeners.contains(listener)) {
        return;
      }

      if (call.method != 'onEvent') throw UnimplementedError();

      String eventName = call.arguments['eventName'];
      listener.onWindowEvent(eventName);
      Map<String, Function> funcMap = {};
      funcMap[eventName]?.call();
    }
  }

  Future<dynamic> invoke(
    String fieldName, {
    Map<String, dynamic>? params,
  }) async {
    final Map<String, dynamic> arguments = {if (params != null) ...params};
    return await _channel.invokeMethod(fieldName, arguments);
  }
}

ConfigData configData = ConfigData._();

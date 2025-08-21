import 'dart:ui';
import 'package:flutter/gestures.dart';
import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';

class AboutPage extends StatefulWidget {
  const AboutPage({super.key});

  @override
  State<AboutPage> createState() => _AboutPageState();
}

class _AboutPageState extends State<AboutPage>
    with SingleTickerProviderStateMixin {
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
      padding: const EdgeInsets.symmetric(vertical: 10),
      children: [
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
          child: _GlassCard(
            child: _InfoCard(
              subtitle:
                  '|界面版本: 1.0.0 |后端版本: 1.0.1 |安装器版本: 1.0.0 |辅助版本: 1.0.2 |SYCL 硬件加速后端: 1.0.0',
              content:
                  '高性能与颜值并存， 多种技术栈混合打造，免费且现代化游戏辅助\n\n'
                  '界面框架: Flutter， WPF\n'
                  '开发语言: Dart， C/C++， DPC++， C#， HTML， CSS， JavaScript\n'
                  '开发工具: Visual Studio Code， Visual Studio Enterprise 2022， ReSharper\n'
                  '其他工具: Intel VTune， Intel Advisor， WinDbg， Procmon64， Sublime Text\n'
                  '工具链: Flutter SDK， Dart SDK， Window SDK， MSVC， intel oneapi DPC++/C++ Compiler 2025， .NET 9 SDK',
            ),
          ),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
          child: _GlassCard(child: _AuthorCard()),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
          child: _GlassCard(child: _QQGroupCard()),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
          child: _GlassCard(child: _DisclaimerCard()),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
          child: _GlassCard(child: _PrivacyPolicyCard()),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 5),
          child: _GlassCard(child: _ServiceAgreementCard()),
        ),
      ],
    );

    var scrollable2 = Listener(
      onPointerSignal: onPointerSignal,
      child: scrollable,
    );

    return SizedBox.expand(
      child: ScrollConfiguration(
        behavior: ScrollConfiguration.of(context).copyWith(scrollbars: false),
        child: scrollable2,
      ),
    );
  }
}

class _InfoCard extends StatelessWidget {
  final String subtitle;
  final String content;
  const _InfoCard({required this.subtitle, required this.content});

  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        Card(
          elevation: 0,
          color: Colors.transparent,
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(25),
          ),
          child: Padding(
            padding: const EdgeInsets.all(24.0),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Row(children: [Image.asset("lib/images/logo.png", scale: 2.5)]),
                const SizedBox(height: 8),
                Text(
                  subtitle,
                  style: TextStyle(fontSize: 16, color: Colors.white),
                ),
                const SizedBox(height: 12),
                Text(
                  content,
                  style: TextStyle(fontSize: 14, color: Colors.white70),
                ),
              ],
            ),
          ),
        ),
        Positioned(
          left: -30,
          top: 8,
          child: Transform.rotate(
            angle: -0.785398,
            child: SizedBox(
              width: 100,
              child: Container(
                padding: const EdgeInsets.symmetric(vertical: 4),
                color: Colors.blue.withOpacity(0.85),
                alignment: Alignment.center,
                child: const Text(
                  'FREE',
                  textAlign: TextAlign.center,
                  style: TextStyle(
                    color: Colors.white,
                    fontWeight: FontWeight.bold,
                    fontSize: 14,
                    letterSpacing: 2,
                    decoration: TextDecoration.none,
                  ),
                ),
              ),
            ),
          ),
        ),
      ],
    );
  }
}

class _AuthorCard extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      color: Colors.transparent,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(25)),
      child: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Row(
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                CircleAvatar(
                  radius: 20,
                  backgroundImage: NetworkImage(
                    'http://q1.qlogo.cn/g?b=qq&nk=1992724048&s=100',
                  ),
                ),
                const SizedBox(width: 12),
                Text('遂沫', style: TextStyle(fontSize: 16, color: Colors.white)),
                const SizedBox(width: 8),
                Text(
                  '|主页: 遂沫.com',
                  style: TextStyle(fontSize: 14, color: Colors.white70),
                ),
                const SizedBox(width: 8),
                Text(
                  '|博客: issuimo.com',
                  style: TextStyle(fontSize: 14, color: Colors.white70),
                ),
                const SizedBox(width: 8),
                Text(
                  '|邮件: 1992724048@qq.com',
                  style: TextStyle(fontSize: 14, color: Colors.white70),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _QQGroupCard extends StatelessWidget {
  const _QQGroupCard();

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      color: Colors.transparent,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(25)),
      child: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                CircleAvatar(
                  radius: 26,
                  backgroundImage: NetworkImage(
                    'https://p.qlogo.cn/gh/838737409/838737409/640/',
                  ),
                ),
                const SizedBox(width: 10),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text(
                      '千禧科技',
                      style: TextStyle(
                        fontSize: 18,
                        color: Colors.white,
                        fontWeight: FontWeight.bold,
                      ),
                    ),
                    const SizedBox(height: 2),
                    const Text(
                      '群号：838737409',
                      style: TextStyle(fontSize: 14, color: Colors.white70),
                    ),
                  ],
                ),
              ],
            ),
            const SizedBox(height: 18),
            Row(
              mainAxisAlignment: MainAxisAlignment.start,
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                ClipRRect(
                  borderRadius: BorderRadius.circular(12),
                  child: Image.asset(
                    'lib/images/qr_qq.jpg',
                    width: 180,
                    height: 180,
                    fit: BoxFit.cover,
                  ),
                ),
                const SizedBox(width: 10),
                ClipRRect(
                  borderRadius: BorderRadius.circular(12),
                  child: Image.asset(
                    'lib/images/qr_qq2.png',
                    width: 180,
                    height: 180,
                    fit: BoxFit.cover,
                  ),
                ),
                const SizedBox(width: 10),
                const Text(
                  '扫码加入QQ群\n群里不定期发布源码和更新包\n(文件太大传Github真不方便)\n备用链接:\n博客:issuimo.com (群没了看这里，搜索标签 "辅助" )',
                  style: TextStyle(
                    fontSize: 14,
                    color: Colors.white70,
                    fontWeight: FontWeight.bold,
                  ),
                ),
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _DisclaimerCard extends StatelessWidget {
  const _DisclaimerCard();

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      color: Colors.transparent,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(25)),
      child: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(
                  Icons.warning_amber_rounded,
                  size: 28,
                  color: Colors.orange[700],
                ),
                const SizedBox(width: 10),
                Text(
                  '免责声明',
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.orange[800],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 10),
            const Text(
              '本软件仅供学习交流、科研等非商业性质的用途，严禁将本软件用于任何商业目的。\n因使用本软件（包括但不限于第三方对本软件的修改、分发等行为）所产生的任何直接或间接后果，均由使用者本人承担，与本软件及其制作者无关。本软件及制作者不对因使用本软件导致的任何损失或法律责任承担任何责任。用户需自行承担使用本软件的风险。',
              style: TextStyle(fontSize: 14, color: Colors.white),
            ),
            const SizedBox(height: 16),
            Row(children: [
                
              ],
            ),
          ],
        ),
      ),
    );
  }
}

class _PrivacyPolicyCard extends StatelessWidget {
  const _PrivacyPolicyCard();

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      color: Colors.transparent,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(25)),
      child: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(Icons.privacy_tip, size: 28, color: Colors.green[700]),
                const SizedBox(width: 10),
                Text(
                  '隐私政策',
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.green[800],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 10),
            const Text(
              '本软件不会主动收集、存储或上传您的任何个人信息，所有信息均为本地离线使用，仅限于程序运行所需的最基本配置信息，不涉及任何个人身份信息。如有第三方修改本软件导致的信息收集、传输等行为，均属于个人行为，与本软件及制作者无关。',
              style: TextStyle(fontSize: 14, color: Colors.white),
            ),
          ],
        ),
      ),
    );
  }
}

class _ServiceAgreementCard extends StatelessWidget {
  const _ServiceAgreementCard();

  @override
  Widget build(BuildContext context) {
    return Card(
      elevation: 0,
      color: Colors.transparent,
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(25)),
      child: Padding(
        padding: const EdgeInsets.all(24.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                Icon(Icons.article, size: 28, color: Colors.blue[700]),
                const SizedBox(width: 10),
                Text(
                  '服务协议',
                  style: TextStyle(
                    fontSize: 18,
                    fontWeight: FontWeight.bold,
                    color: Colors.blue[800],
                  ),
                ),
              ],
            ),
            const SizedBox(height: 10),
            const Text(
              '1. 用户在使用本软件前应仔细阅读并同意本服务协议。\n2. 本软件为免费产品，开发者有权随时对软件进行更新、升级或终止服务，无需另行通知。\n3. 用户不得利用本软件从事任何违法违规活动，若因用户行为导致的法律责任，由用户自行承担。\n4. 本协议未尽事宜，开发者保留最终解释权。',
              style: TextStyle(fontSize: 14, color: Colors.white),
            ),
          ],
        ),
      ),
    );
  }
}

class _GlassCard extends StatelessWidget {
  final Widget child;
  const _GlassCard({required this.child});

  @override
  Widget build(BuildContext context) {
    return ClipRRect(
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
          child: child,
        ),
      ),
    );
  }
}

import 'dart:ui';
import 'package:flutter/material.dart';

class HomePage extends StatefulWidget {
  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage>
    with AutomaticKeepAliveClientMixin<HomePage> {
  @override
  Widget build(BuildContext context) {
    super.build(context);
    return Stack(
      children: [
        Positioned(left: 20, bottom: 20, child: NoticeBoard()),
        Positioned(left: 380, bottom: 20, child: CheckUpdateButton()),
      ],
    );
  }

  @override
  bool get wantKeepAlive => true;
}

class CheckUpdateButton extends StatelessWidget {
  const CheckUpdateButton({super.key});

  @override
  Widget build(BuildContext context) {
    return ClipRRect(
      borderRadius: BorderRadius.circular(10),
      child: BackdropFilter(
        filter: ImageFilter.blur(sigmaX: 2, sigmaY: 2),
        child: Container(
          width: 140,
          height: 40,
          decoration: BoxDecoration(
            color: Colors.white.withOpacity(0.25),
            borderRadius: BorderRadius.circular(10),
            boxShadow: [
              BoxShadow(
                color: Colors.black.withOpacity(0.10),
                blurRadius: 16,
                offset: Offset(0, 4),
              ),
            ],
          ),
          child: ElevatedButton(
            onPressed: () {},
            style: ElevatedButton.styleFrom(
              backgroundColor: Colors.transparent,
              shadowColor: Colors.transparent,
              shape: RoundedRectangleBorder(
                borderRadius: BorderRadius.circular(10),
              ),
            ),
            child: Center(
              child: Row(
                children: [
                  Icon(
                    Icons.update,
                    color: Colors.white.withOpacity(0.85),
                    size: 20,
                  ),
                  SizedBox(width: 5),
                  Text(
                    '检查更新',
                    style: TextStyle(
                      fontSize: 16,
                      fontFamily: 'ui_font',
                      color: Colors.white,
                      decoration: TextDecoration.none,
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}

class NoticeBoard extends StatefulWidget {
  @override
  State<NoticeBoard> createState() => _NoticeBoardState();
}

class _NoticeBoardState extends State<NoticeBoard>
    with
        SingleTickerProviderStateMixin,
        AutomaticKeepAliveClientMixin<NoticeBoard> {
  int _tabIndex = 0;
  final List<String> tabs = ['活动', '公告', '资讯'];
  final List<List<Map<String, String>>> tabContents = [
    [
      {'title': '活动1', 'date': '06/05'},
      {'title': '活动2', 'date': '06/04'},
    ],
    [
      {'title': '公告1', 'date': '06/03'},
      {'title': '公告2', 'date': '06/01'},
      {'title': '公告2', 'date': '06/01'},
    ],
    [
      {'title': '资讯1', 'date': '05/30'},
      {'title': '资讯1', 'date': '05/30'},
      {'title': '资讯1', 'date': '05/30'},
      {'title': '资讯2', 'date': '05/28'},
    ],
  ];

  @override
  bool get wantKeepAlive => true;

  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return ClipRRect(
      borderRadius: BorderRadius.circular(10),
      child: BackdropFilter(
        filter: ImageFilter.blur(sigmaX: 2, sigmaY: 2),
        child: Container(
          width: 340,
          decoration: BoxDecoration(
            borderRadius: BorderRadius.circular(10),
            gradient: LinearGradient(
              begin: Alignment.topLeft,
              end: Alignment.bottomRight,
              colors: [
                Colors.white.withOpacity(0.10),
                Colors.blue.withOpacity(0.10),
              ],
              stops: [0.0, 1.0],
            ),
            boxShadow: [
              BoxShadow(
                color: Colors.black.withOpacity(0.10),
                blurRadius: 16,
                offset: Offset(0, 4),
              ),
            ],
          ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              Container(
                height: 200,
                width: 340,
                decoration: BoxDecoration(
                  boxShadow: [
                    BoxShadow(
                      color: Colors.black.withOpacity(0.15),
                      blurRadius: 10,
                      offset: Offset(0, 4),
                    ),
                  ],
                  borderRadius: BorderRadius.circular(10),
                  image: DecorationImage(
                    image: AssetImage('lib/images/bg2.jpeg'),
                    fit: BoxFit.cover,
                    alignment: Alignment.topCenter,
                  ),
                ),
              ),
              Padding(
                padding: const EdgeInsets.symmetric(
                  horizontal: 12,
                  vertical: 2,
                ),
                child: Row(
                  children: List.generate(tabs.length, (i) {
                    final selected = _tabIndex == i;
                    return GestureDetector(
                      onTap: () => setState(() => _tabIndex = i),
                      child: AnimatedContainer(
                        duration: Duration(milliseconds: 500),
                        padding: const EdgeInsets.symmetric(
                          horizontal: 16,
                          vertical: 10,
                        ),
                        margin: EdgeInsets.only(right: 8, top: 6, bottom: 2),
                        decoration: BoxDecoration(
                          color: selected
                              ? Colors.blue.withOpacity(0.18)
                              : Colors.transparent,
                          borderRadius: BorderRadius.circular(12),
                          border: selected
                              ? Border.all(
                                  color: Colors.blue.withOpacity(0.45),
                                  width: 1.2,
                                )
                              : Border.all(
                                  color: Colors.transparent,
                                  width: 1.2,
                                ),
                        ),
                        child: Text(
                          tabs[i],
                          style: TextStyle(
                            fontSize: 16,
                            fontFamily: 'ui_font',
                            fontWeight: selected
                                ? FontWeight.bold
                                : FontWeight.normal,
                            color: selected ? Colors.blueAccent : Colors.white,
                            decoration: TextDecoration.none,
                          ),
                        ),
                      ),
                    );
                  }),
                ),
              ),
              Divider(
                height: 1,
                thickness: 1,
                color: Colors.white.withOpacity(0.3),
              ),
              AnimatedSize(
                duration: Duration(milliseconds: 500),
                curve: Curves.fastEaseInToSlowEaseOut,
                alignment: Alignment.topCenter,
                child: Padding(
                  padding: const EdgeInsets.symmetric(
                    horizontal: 16,
                    vertical: 8,
                  ),
                  child: Column(
                    children: List.generate(tabContents[_tabIndex].length, (i) {
                      final item = tabContents[_tabIndex][i];
                      return Padding(
                        padding: const EdgeInsets.symmetric(vertical: 4),
                        child: Row(
                          children: [
                            Container(
                              width: 6,
                              height: 6,
                              margin: EdgeInsets.only(right: 8, left: 2),
                              decoration: BoxDecoration(
                                color: Colors.blueAccent,
                                shape: BoxShape.circle,
                              ),
                            ),
                            Expanded(
                              child: Text(
                                item['title']!,
                                style: TextStyle(
                                  fontSize: 15,
                                  fontFamily: 'ui_font',
                                  color: Colors.white,
                                  decoration: TextDecoration.none,
                                ),
                                maxLines: 1,
                                overflow: TextOverflow.ellipsis,
                              ),
                            ),
                            SizedBox(width: 10),
                            Text(
                              item['date']!,
                              style: TextStyle(
                                fontSize: 13,
                                fontFamily: 'ui_font',
                                color: Colors.grey[300],
                                decoration: TextDecoration.none,
                              ),
                            ),
                          ],
                        ),
                      );
                    }),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

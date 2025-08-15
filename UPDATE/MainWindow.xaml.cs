namespace UPDATE {
    using System.Diagnostics;
    using System.IO;
    using System.Net;
    using System.Net.Http;
    using System.Text.Json;
    using System.Windows;

    using SharpCompress.Archives;
    using SharpCompress.Common;

    public class VerDat {
        public string ToolsBox { get; }
        public string GUI { get; }
        public string SYCL { get; }
        public string UPDATE { get; }
    }

    /// <summary>
    ///     Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        readonly HttpClient _http = new();
        bool gui_dw;
        int max_dw;
        int min_dw;
        bool sycl_dw;
        bool toolsbox_dw;
        bool update_dw;


        public MainWindow() {
            this.InitializeComponent();
            MainWindow.del_temp();
        }

        static void del_temp() {
            if (File.Exists("update.7z")) {
                File.Delete("update.7z");
            }
        }

        void MainWindow_OnLoaded(object sender, RoutedEventArgs e) {
            new Thread(() => {
                Thread.Sleep(1000);
                try {
                    if (File.Exists("ver.json")) {
                        this.update_check();
                    } else {
                        this.toolsbox_dw = true;
                        this.gui_dw = true;
                        this.sycl_dw = true;
                        this.update_dw = true;
                        this.max_dw = 4;
                    }

                    this.download_file();
                } catch (Exception exception) {
                    MessageBox.Show(messageBoxText: exception.ToString(), caption: "致命错误!", button: MessageBoxButton.OK, icon: MessageBoxImage.Error);
                    throw;
                }
            }).Start();
        }

        void download_file() {
            if (this.toolsbox_dw) {
                this.min_dw++;
                this.download_toolsbox();
            }

            if (this.gui_dw) {
                this.min_dw++;
                download_gui();
            }

            if (this.sycl_dw) {
                this.min_dw++;
                download_sycl();
            }

            if (this.update_dw) {
                this.min_dw++;
                download_cheat();
            }

            if (File.Exists("ver.json")) {
                File.Delete("ver.json");
            }

            this.SetTips("更新完成!");
            File.WriteAllText(path: "ver.json", contents: this._http.GetStringAsync("https://1992724048.github.io/SausageManCheat/ver.json").GetAwaiter().GetResult());

            Thread.Sleep(5000);
            Environment.Exit(0);
        }

        void download_toolsbox() {
            this.DownloadAsync(url: "https://1992724048.github.io/SausageManCheat/ToolsBox.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载启动器...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }
        
        void download_gui() {
            this.DownloadAsync(url: "https://1992724048.github.io/SausageManCheat/gui.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载启动器界面...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }

        void download_sycl() {
            this.DownloadAsync(url: "https://1992724048.github.io/SausageManCheat/sycl.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载硬件加速...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }
        
        void download_cheat() {
            this.DownloadAsync(url: "https://1992724048.github.io/SausageManCheat/cheat.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载辅助...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }

        async Task DownloadAsync(string url, string filePath, string title, CancellationToken token) {
            this.SetTips("步骤：正在获取文件信息…");
            using HttpResponseMessage res = await this._http.GetAsync(requestUri: url, completionOption: HttpCompletionOption.ResponseHeadersRead, cancellationToken: token);
            res.EnsureSuccessStatusCode();

            long total = res.Content.Headers.ContentLength ?? 0;
            if (total == 0) {
                throw new("无法获取文件大小");
            }

            long received = 0;
            var buf = new byte[81920];
            var sw = Stopwatch.StartNew();

            await using var fs = new FileStream(path: filePath, mode: FileMode.Create, access: FileAccess.Write, share: FileShare.None, bufferSize: buf.Length, useAsync: true);
            await using Stream net = await res.Content.ReadAsStreamAsync(token);

            int len;
            while ((len = await net.ReadAsync(buffer: buf, offset: 0, count: buf.Length, cancellationToken: token)) > 0) {
                await fs.WriteAsync(buffer: buf, offset: 0, count: len, cancellationToken: token);
                received += len;

                double speed = received / sw.Elapsed.TotalSeconds;
                var eta = TimeSpan.FromSeconds((total - received) / speed);

                this.Dispatcher.Invoke(() => {
                    this.ProgressBar.Maximum = 100.0f;
                    this.ProgressBar.Value = received / (float)total * 100.0f;
                    this.Tips.Content = $"步骤：{title} " + $"总大小：{MainWindow.Bytes(total)} " + $"已下载：{MainWindow.Bytes(received)} " + $"剩余时间：{eta:hh\\:mm\\:ss}";
                });
            }
        }

        async Task Extract7zAsync(string archivePath, string outDir, string password = "") {
            /*this.SetTips("步骤：正在解压…");

            await Task.Run(() => {
                Directory.CreateDirectory(outDir);

                var options = new ExtractionOptions { ExtractFullPath = true, Overwrite = true };

                using IArchive archive = ArchiveFactory.Open(filePath: archivePath, options: new() { Password = password });

                foreach (IArchiveEntry entry in archive.Entries) {
                    if (!entry.IsDirectory) {
                        entry.WriteToDirectory(destinationDirectory: outDir, options: options);
                    }
                }
            });

            if (File.Exists(archivePath)) {
                File.Delete(archivePath);
            }

            this.SetTips("解压完成！");*/
        }


        void SetTips(string txt) => this.Dispatcher.Invoke(() => this.Tips.Content = txt);

        static string Bytes(long b) => b switch
                                       {
                                       < 1024       => $"{b} B",
                                       < 1024 * 1024 => $"{b / 1024.0:F1} KB",
                                       < 1024L * 1024 * 1024 => $"{b / (1024.0 * 1024):F1} MB",
                                       var _ => $"{b / (1024.0 * 1024 * 1024):F1} GB",
                                       };

        void update_check() {
            var json = File.ReadAllText("ver.json");
            VerDat ver = JsonSerializer.Deserialize<VerDat>(json) ?? throw new SystemException("解析版本失败! 请重新安装该程序!");
            VerDat remote = this.htpp_data();

            if (!this.check_ver(ver: ver.ToolsBox, ver_rm: remote.ToolsBox)) {
                this.toolsbox_dw = true;
            }
            if (!this.check_ver(ver: ver.GUI, ver_rm: remote.GUI)) {
                this.gui_dw = true;
            }
            if (!this.check_ver(ver: ver.SYCL, ver_rm: remote.SYCL)) {
                this.sycl_dw = true;
            }
            if (!this.check_ver(ver: ver.UPDATE, ver_rm: remote.UPDATE)) {
                this.update_dw = true;
            }
        }

        VerDat htpp_data() {
            this._http.DefaultRequestHeaders.UserAgent.ParseAdd("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36");
            this._http.DefaultRequestVersion = HttpVersion.Version11;
            string json = this._http.GetStringAsync("https://1992724048.github.io/SausageManCheat/ver.json").GetAwaiter().GetResult();
            VerDat? remote = JsonSerializer.Deserialize<VerDat>(json);

            if (remote is null) {
                throw new SystemException("获取最新版本失败!");
            }

            return remote;
        }

        bool check_ver(string ver, string ver_rm) {
            if (ver == ver_rm) {
                return true;
            }

            this.max_dw++;
            return false;
        }
    }
}

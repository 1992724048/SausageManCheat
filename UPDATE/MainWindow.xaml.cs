namespace UPDATE {
    using System.Diagnostics;
    using System.IO;
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
            File.WriteAllText(path: "ver.json", contents: this._http.GetStringAsync("https://raw.githubusercontent.com/1992724048/SausageManCheat/refs/heads/master/ver.json").GetAwaiter().GetResult());

            Thread.Sleep(5000);
            Environment.Exit(0);
        }

        void download_toolsbox() {
            this.DownloadAsync(url: "https://raw.githubusercontent.com/1992724048/SausageManCheat/refs/heads/master/ToolsBox.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载启动器...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }
        
        void download_gui() {
            this.DownloadAsync(url: "https://raw.githubusercontent.com/1992724048/SausageManCheat/refs/heads/master/gui.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载启动器界面...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }

        void download_sycl() {
            this.DownloadAsync(url: "https://raw.githubusercontent.com/1992724048/SausageManCheat/refs/heads/master/sycl.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载硬件加速...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./").Wait();
        }
        
        void download_cheat() {
            this.DownloadAsync(url: "https://raw.githubusercontent.com/1992724048/SausageManCheat/refs/heads/master/cheat.7z", filePath: "update.7z", title: $"({this.min_dw}/{this.max_dw}) 正在下载硬件加速...", token: CancellationToken.None).Wait();
            this.Extract7zAsync(archivePath: "update.7z", outDir: "./GamePlusPlus").Wait();
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
                    this.ProgressBar.Maximum = 100;
                    this.ProgressBar.Value = received / total * 100.0f;
                    this.Tips.Content = $"步骤：{title} " + $"总大小：{MainWindow.Bytes(total)} " + $"已下载：{MainWindow.Bytes(received)} " + $"剩余时间：{eta:hh\\:mm\\:ss}";
                });
            }
        }

        async Task Extract7zAsync(string archivePath, string outDir, string password = "") {
            this.SetTips("步骤：正在解压…");

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

            this.SetTips("解压完成！");
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
            VerDat ver = JsonSerializer.Deserialize<VerDat>(File.ReadAllText("ver.json")) ?? throw new SystemException("解析版本失败! 请重新安装该程序!");
            var tb = Version.Parse(ver.ToolsBox);
            var gui = Version.Parse(ver.GUI);
            var sycl = Version.Parse(ver.SYCL);
            var upd = Version.Parse(ver.UPDATE);

            VerDat remote = this.htpp_data();
            var tb_rm = Version.Parse(remote.ToolsBox);
            var gui_rm = Version.Parse(remote.GUI);
            var sycl_rm = Version.Parse(remote.SYCL);
            var upd_rm = Version.Parse(remote.UPDATE);

            if (!this.check_ver(ver: tb, ver_rm: tb_rm)) {
                this.toolsbox_dw = true;
            }
            if (!this.check_ver(ver: gui, ver_rm: gui_rm)) {
                this.gui_dw = true;
            }
            if (!this.check_ver(ver: sycl, ver_rm: sycl_rm)) {
                this.sycl_dw = true;
            }
            if (!this.check_ver(ver: upd, ver_rm: upd_rm)) {
                this.update_dw = true;
            }
        }

        VerDat htpp_data() {
            var http = new HttpClient();
            string json = this._http.GetStringAsync("https://raw.githubusercontent.com/1992724048/SausageManCheat/refs/heads/master/ver.json").GetAwaiter().GetResult();
            VerDat? remote = JsonSerializer.Deserialize<VerDat>(json);

            if (remote is null) {
                throw new SystemException("获取最新版本失败!");
            }

            return remote;
        }

        bool check_ver(Version ver, Version ver_rm) {
            if (ver == ver_rm) {
                return true;
            }

            this.max_dw++;
            return false;
        }
    }
}

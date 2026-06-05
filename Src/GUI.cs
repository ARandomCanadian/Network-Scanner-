using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Web.Script.Serialization;
using System.Windows.Forms;

// Matches the top-level JSON object saved by the C++ ReportManager.
public class ScanReport
{
    public List<HostRecord> hosts { get; set; }
}

// Represents one host loaded from scan_results.json.
public class HostRecord
{
    public string ip { get; set; }
    public string hostname { get; set; }
    public bool online { get; set; }
    public List<PortRecord> openPorts { get; set; }
}

// Represents one open port loaded from scan_results.json.
public class PortRecord
{
    public int port { get; set; }
    public string service { get; set; }
    public string banner { get; set; }
    public string state { get; set; }
}

// Main WinForms window for the scanner GUI.
// This GUI stays separate from the C++ scanner and launches NetworkScanner.exe when scanning.
public class ScannerGui : Form
{
    // Controls used for user input, buttons, result tables, logs, and status messages.
    private readonly TextBox subnetTextBox = new TextBox();
    private readonly TextBox scannerExeTextBox = new TextBox();
    private readonly Button browseButton = new Button();
    private readonly Button scanButton = new Button();
    private readonly Button loadButton = new Button();
    private readonly Button sortButton = new Button();
    private readonly Button searchButton = new Button();
    private readonly TextBox searchIpTextBox = new TextBox();
    private readonly NumericUpDown searchPortInput = new NumericUpDown();
    private readonly DataGridView hostsGrid = new DataGridView();
    private readonly DataGridView portsGrid = new DataGridView();
    private readonly TextBox logBox = new TextBox();
    private readonly Label statusLabel = new Label();

    // Stores the most recently loaded scan report so sorting/searching can use it.
    private ScanReport currentReport = new ScanReport { hosts = new List<HostRecord>() };

    // Sets up the window, builds the controls, connects events, and finds the scanner EXE.
    public ScannerGui()
    {
        Text = "Educational Network Scanner GUI";
        Width = 1100;
        Height = 760;
        StartPosition = FormStartPosition.CenterScreen;
        MinimumSize = new Size(900, 650);

        BuildLayout();
        WireEvents();
        scannerExeTextBox.Text = FindDefaultScannerExePath();
    }

    // Creates the GUI layout entirely in code instead of using the Visual Studio Designer.
    private void BuildLayout()
    {
        // Main layout divides the form into controls, host results, port results, log, and status.
        var main = new TableLayoutPanel();
        main.Dock = DockStyle.Fill;
        main.RowCount = 5;
        main.ColumnCount = 1;
        main.Padding = new Padding(10);
        main.RowStyles.Add(new RowStyle(SizeType.Absolute, 80));
        main.RowStyles.Add(new RowStyle(SizeType.Percent, 45));
        main.RowStyles.Add(new RowStyle(SizeType.Percent, 35));
        main.RowStyles.Add(new RowStyle(SizeType.Absolute, 130));
        main.RowStyles.Add(new RowStyle(SizeType.Absolute, 28));
        Controls.Add(main);

        // Top layout contains the subnet input, scanner EXE path, and action buttons.
        var top = new TableLayoutPanel();
        top.Dock = DockStyle.Fill;
        top.ColumnCount = 7;
        top.RowCount = 2;
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 95));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 28));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 95));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 45));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90));
        top.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute, 90));
        main.Controls.Add(top, 0, 0);

        top.Controls.Add(new Label { Text = "Subnet", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft }, 0, 0);
        subnetTextBox.Text = "192.168.56.";
        subnetTextBox.Dock = DockStyle.Fill;
        top.Controls.Add(subnetTextBox, 1, 0);

        top.Controls.Add(new Label { Text = "Scanner EXE", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft }, 2, 0);
        scannerExeTextBox.Dock = DockStyle.Fill;
        top.Controls.Add(scannerExeTextBox, 3, 0);

        browseButton.Text = "Browse";
        browseButton.Dock = DockStyle.Fill;
        top.Controls.Add(browseButton, 4, 0);

        scanButton.Text = "Run Scan";
        scanButton.Dock = DockStyle.Fill;
        top.Controls.Add(scanButton, 5, 0);

        loadButton.Text = "Load JSON";
        loadButton.Dock = DockStyle.Fill;
        top.Controls.Add(loadButton, 6, 0);

        top.Controls.Add(new Label { Text = "Search IP", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft }, 0, 1);
        searchIpTextBox.Dock = DockStyle.Fill;
        top.Controls.Add(searchIpTextBox, 1, 1);

        top.Controls.Add(new Label { Text = "Search Port", Dock = DockStyle.Fill, TextAlign = ContentAlignment.MiddleLeft }, 2, 1);
        searchPortInput.Minimum = 1;
        searchPortInput.Maximum = 65535;
        searchPortInput.Value = 80;
        searchPortInput.Dock = DockStyle.Fill;
        top.Controls.Add(searchPortInput, 3, 1);

        sortButton.Text = "Sort Hosts";
        sortButton.Dock = DockStyle.Fill;
        top.Controls.Add(sortButton, 4, 1);

        searchButton.Text = "Binary Search";
        searchButton.Dock = DockStyle.Fill;
        top.Controls.Add(searchButton, 5, 1);

        // Host grid shows discovered devices and how many open ports each has.
        ConfigureGrid(hostsGrid);
        hostsGrid.SelectionMode = DataGridViewSelectionMode.FullRowSelect;
        hostsGrid.MultiSelect = false;
        hostsGrid.AutoGenerateColumns = false;
        hostsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "IP", DataPropertyName = "ip", Width = 160 });
        hostsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "Host Name", DataPropertyName = "hostname", Width = 180 });
        hostsGrid.Columns.Add(new DataGridViewCheckBoxColumn { HeaderText = "Online", DataPropertyName = "online", Width = 80 });
        hostsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "Open Ports", Name = "OpenPortCount", Width = 100 });
        main.Controls.Add(hostsGrid, 0, 1);

        // Port grid shows the open ports for the selected host.
        ConfigureGrid(portsGrid);
        portsGrid.AutoGenerateColumns = false;
        portsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "Port", DataPropertyName = "port", Width = 80 });
        portsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "Service", DataPropertyName = "service", Width = 130 });
        portsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "State", DataPropertyName = "state", Width = 110 });
        portsGrid.Columns.Add(new DataGridViewTextBoxColumn { HeaderText = "Banner", DataPropertyName = "banner", AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill });
        main.Controls.Add(portsGrid, 0, 2);

        // Log box displays output from the C++ scanner process.
        logBox.Dock = DockStyle.Fill;
        logBox.Multiline = true;
        logBox.ScrollBars = ScrollBars.Vertical;
        logBox.ReadOnly = true;
        main.Controls.Add(logBox, 0, 3);

        statusLabel.Text = "Ready";
        statusLabel.Dock = DockStyle.Fill;
        statusLabel.TextAlign = ContentAlignment.MiddleLeft;
        main.Controls.Add(statusLabel, 0, 4);
    }

    // Applies the same read-only settings to both result grids.
    private void ConfigureGrid(DataGridView grid)
    {
        grid.Dock = DockStyle.Fill;
        grid.ReadOnly = true;
        grid.AllowUserToAddRows = false;
        grid.AllowUserToDeleteRows = false;
        grid.RowHeadersVisible = false;
        grid.AutoSizeColumnsMode = DataGridViewAutoSizeColumnsMode.None;
    }

    // Connects button clicks and table selection changes to their methods.
    private void WireEvents()
    {
        browseButton.Click += BrowseButton_Click;
        scanButton.Click += async (sender, args) => await RunScanAsync();
        loadButton.Click += (sender, args) => LoadReportFromDefaultLocation();
        sortButton.Click += (sender, args) => SortHostsByOpenPorts();
        searchButton.Click += (sender, args) => SearchForPort();
        hostsGrid.SelectionChanged += (sender, args) => ShowSelectedHostPorts();
        hostsGrid.CellFormatting += HostsGrid_CellFormatting;
    }

    // Searches common folders for NetworkScanner.exe so the user may not need to browse manually.
    private string FindDefaultScannerExePath()
    {
        string guiFolder = AppDomain.CurrentDomain.BaseDirectory;
        string parentFolder = Directory.GetParent(guiFolder.TrimEnd(Path.DirectorySeparatorChar)) == null
            ? guiFolder
            : Directory.GetParent(guiFolder.TrimEnd(Path.DirectorySeparatorChar)).FullName;

        string[] possiblePaths =
        {
            Path.Combine(guiFolder, "NetworkScanner.exe"),
            Path.Combine(guiFolder, "build", "NetworkScanner.exe"),
            Path.Combine(parentFolder, "NetworkScanner.exe"),
            Path.Combine(parentFolder, "build", "NetworkScanner.exe"),
            Path.Combine(Directory.GetCurrentDirectory(), "NetworkScanner.exe"),
            Path.Combine(Directory.GetCurrentDirectory(), "build", "NetworkScanner.exe")
        };

        foreach (string path in possiblePaths)
        {
            if (File.Exists(path))
                return path;
        }

        return Path.Combine(guiFolder, "NetworkScanner.exe");
    }

    // Opens a file picker so the user can select the C++ scanner executable.
    private void BrowseButton_Click(object sender, EventArgs e)
    {
        using (var dialog = new OpenFileDialog())
        {
            dialog.Filter = "Executable files (*.exe)|*.exe|All files (*.*)|*.*";
            dialog.Title = "Select NetworkScanner.exe";

            if (dialog.ShowDialog() == DialogResult.OK)
                scannerExeTextBox.Text = dialog.FileName;
        }
    }

    // Launches NetworkScanner.exe in GUI scan mode and waits for it to finish.
    // Running this asynchronously keeps the GUI from freezing during the scan.
    private async System.Threading.Tasks.Task RunScanAsync()
    {
        string scannerPath = scannerExeTextBox.Text.Trim();
        string subnet = subnetTextBox.Text.Trim();

        if (!File.Exists(scannerPath))
        {
            MessageBox.Show("Could not find NetworkScanner.exe. Build the C++ scanner first or click Browse.", "Missing Scanner EXE");
            return;
        }

        if (string.IsNullOrWhiteSpace(subnet))
        {
            MessageBox.Show("Enter a subnet like 192.168.56.", "Missing Subnet");
            return;
        }

        // Disable buttons while scanning so the user cannot start multiple scans at once.
        SetBusy(true);
        logBox.Clear();
        AppendLog("Starting GUI scan...\r\n");

        try
        {
            string workingDirectory = Path.GetDirectoryName(scannerPath);

            // ProcessStartInfo controls how the GUI starts the C++ scanner EXE.
            var startInfo = new ProcessStartInfo();
            startInfo.FileName = scannerPath;
            startInfo.Arguments = "--gui-scan \"" + subnet + "\"";
            startInfo.WorkingDirectory = workingDirectory;
            startInfo.UseShellExecute = false;
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError = true;
            startInfo.CreateNoWindow = true;

            using (var process = new Process())
            {
                process.StartInfo = startInfo;
                // Send console output from the scanner into the GUI log box.
                process.OutputDataReceived += (sender, e) => { if (e.Data != null) AppendLog(e.Data + "\r\n"); };
                process.ErrorDataReceived += (sender, e) => { if (e.Data != null) AppendLog("ERROR: " + e.Data + "\r\n"); };

                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                await System.Threading.Tasks.Task.Run(() => process.WaitForExit());

                if (process.ExitCode != 0)
                {
                    MessageBox.Show("The scanner exited with code " + process.ExitCode + ". Check the log box.", "Scanner Error");
                    return;
                }
            }

            // After the scanner exits, load the JSON report it created.
            LoadReport(Path.Combine(workingDirectory, "scan_results.json"));
            statusLabel.Text = "Scan complete";
        }
        catch (Exception ex)
        {
            MessageBox.Show(ex.Message, "GUI Error");
        }
        finally
        {
            SetBusy(false);
        }
    }

    // Loads scan_results.json from beside the scanner EXE, or lets the user choose it manually.
    private void LoadReportFromDefaultLocation()
    {
        string scannerPath = scannerExeTextBox.Text.Trim();
        string folder = File.Exists(scannerPath) ? Path.GetDirectoryName(scannerPath) : Directory.GetCurrentDirectory();
        string reportPath = Path.Combine(folder, "scan_results.json");

        if (!File.Exists(reportPath))
        {
            using (var dialog = new OpenFileDialog())
            {
                dialog.Filter = "JSON files (*.json)|*.json|All files (*.*)|*.*";
                dialog.Title = "Open scan_results.json";

                if (dialog.ShowDialog() != DialogResult.OK)
                    return;

                reportPath = dialog.FileName;
            }
        }

        LoadReport(reportPath);
    }

    // Reads JSON from disk and converts it into C# objects for the GUI tables.
    private void LoadReport(string path)
    {
        string json = File.ReadAllText(path);
        // JavaScriptSerializer is used because it is included with the .NET Framework compiler.
        var serializer = new JavaScriptSerializer();
        serializer.MaxJsonLength = int.MaxValue;
        currentReport = serializer.Deserialize<ScanReport>(json) ?? new ScanReport();

        if (currentReport.hosts == null)
            currentReport.hosts = new List<HostRecord>();

        foreach (HostRecord host in currentReport.hosts)
        {
            if (host.openPorts == null)
                host.openPorts = new List<PortRecord>();
        }

        BindHosts();
        AppendLog("Loaded report: " + path + "\r\n");
        statusLabel.Text = "Loaded " + currentReport.hosts.Count + " host(s)";
    }

    // Refreshes the host grid after loading or sorting results.
    private void BindHosts()
    {
        hostsGrid.DataSource = null;
        hostsGrid.DataSource = currentReport.hosts;

        for (int i = 0; i < currentReport.hosts.Count; i++)
            hostsGrid.Rows[i].Cells["OpenPortCount"].Value = currentReport.hosts[i].openPorts.Count;

        ShowSelectedHostPorts();
    }

    // Shows the ports belonging to the currently selected host.
    private void ShowSelectedHostPorts()
    {
        HostRecord host = GetSelectedHost();
        portsGrid.DataSource = null;

        if (host == null)
            return;

        searchIpTextBox.Text = host.ip;
        portsGrid.DataSource = host.openPorts.OrderBy(port => port.port).ToList();
    }

    // Gets the HostRecord attached to the selected grid row.
    private HostRecord GetSelectedHost()
    {
        if (hostsGrid.CurrentRow == null)
            return null;

        return hostsGrid.CurrentRow.DataBoundItem as HostRecord;
    }

    // Sorts the loaded hosts by most open ports for easier analysis.
    private void SortHostsByOpenPorts()
    {
        if (currentReport.hosts == null)
            return;

        currentReport.hosts = currentReport.hosts
            .OrderByDescending(host => host.openPorts == null ? 0 : host.openPorts.Count)
            .ToList();

        BindHosts();
        statusLabel.Text = "Sorted hosts by most open ports";
    }

    // Searches the selected host/IP for a specific open port using binary search.
    private void SearchForPort()
    {
        string ip = searchIpTextBox.Text.Trim();
        int targetPort = (int)searchPortInput.Value;

        HostRecord host = currentReport.hosts.FirstOrDefault(h => h.ip == ip);
        if (host == null)
        {
            MessageBox.Show("Host " + ip + " was not found in the loaded results.", "Host Not Found");
            return;
        }

        // Binary search requires the ports to be sorted by port number first.
        List<PortRecord> sortedPorts = host.openPorts.OrderBy(port => port.port).ToList();
        int index = BinarySearchPort(sortedPorts, targetPort);

        if (index >= 0)
        {
            PortRecord found = sortedPorts[index];
            MessageBox.Show("Port " + targetPort + " is OPEN on " + ip + "\nService: " + found.service + "\nBanner: " + found.banner, "Port Found");
            portsGrid.DataSource = sortedPorts;
            portsGrid.ClearSelection();
            portsGrid.Rows[index].Selected = true;
            portsGrid.CurrentCell = portsGrid.Rows[index].Cells[0];
        }
        else
        {
            MessageBox.Show("Port " + targetPort + " is not listed as open on " + ip + ".", "Port Not Found");
        }
    }

    // Binary search implementation for finding a port in the sorted port list.
    private int BinarySearchPort(List<PortRecord> ports, int targetPort)
    {
        int low = 0;
        int high = ports.Count - 1;

        while (low <= high)
        {
            // Safer midpoint calculation than (low + high) / 2.
            int mid = low + (high - low) / 2;

            if (ports[mid].port == targetPort)
                return mid;

            if (ports[mid].port < targetPort)
                low = mid + 1;
            else
                high = mid - 1;
        }

        return -1;
    }

    // Fills the calculated Open Ports column because it is not directly stored in JSON.
    private void HostsGrid_CellFormatting(object sender, DataGridViewCellFormattingEventArgs e)
    {
        if (hostsGrid.Columns[e.ColumnIndex].Name == "OpenPortCount" && e.RowIndex >= 0 && e.RowIndex < currentReport.hosts.Count)
            e.Value = currentReport.hosts[e.RowIndex].openPorts.Count;
    }

    // Adds text to the log box safely, even when called from the scanner process thread.
    private void AppendLog(string text)
    {
        if (logBox.InvokeRequired)
        {
            logBox.BeginInvoke(new Action<string>(AppendLog), text);
            return;
        }

        logBox.AppendText(text);
    }

    // Enables or disables GUI controls depending on whether a scan is running.
    private void SetBusy(bool busy)
    {
        scanButton.Enabled = !busy;
        loadButton.Enabled = !busy;
        browseButton.Enabled = !busy;
        sortButton.Enabled = !busy;
        searchButton.Enabled = !busy;
        statusLabel.Text = busy ? "Scanner is running..." : "Ready";
    }

    // Entry point for the GUI executable.
    [STAThread]
    public static void Main()
    {
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);
        Application.Run(new ScannerGui());
    }
}

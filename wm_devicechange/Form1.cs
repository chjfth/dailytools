using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Management;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace wm_devicechange
{
    public partial class Form1 : Form
    {
        static DateTime s_last_DateTime = new DateTime(0);
        //
        static string text_elapse(int millisec)
        {
            if (millisec >= 1000)
            {
                double sec = (double)millisec / 1000;
                return $"(+{sec:g}s)";
            }
            else if (millisec > 0) return $"(+{millisec}ms)";
            else return "";
        }
        //
        void Log(string s)
        {
            DateTime nowdt = DateTime.Now;
            int millisec_gap = (int)(nowdt - s_last_DateTime).TotalMilliseconds;
            int seconds_gap = millisec_gap / 1000;
            if (s_last_DateTime.Ticks > 0 && seconds_gap > 0)
            {
                Log_ui("".PadLeft(Math.Min(10, seconds_gap), '.'));
            }
            string tsprefix = string.Format("[{0:D2}:{1:D2}:{2:D2}.{3:D3}{4}]",
                nowdt.Hour, nowdt.Minute, nowdt.Second, nowdt.Millisecond, text_elapse(millisec_gap)
            );

            Log_ui($"{tsprefix}{s}"); //Console.Out.WriteLine($"{tsprefix}{s}");

            s_last_DateTime = nowdt;
        }

        private List<Device> _DeviceList = new List<Device>();
        private static object locker = new object();
        public Form1()
        {
            InitializeComponent();
        }
        public const Int32 WM_DEVICECHANGE = 0x219;

        protected override void DefWndProc(ref Message m)
        {
            base.DefWndProc(ref m);

            if (m.Msg == WM_DEVICECHANGE)
            {
                lock (locker)
                {
                    var newDeviceList = EnumDevices();

                    List<Device> temp = new List<Device>();
                    foreach (var device in _DeviceList)
                    {
                        if (!newDeviceList.Exists(d => d.Path == device.Path))
                        {
                            device.Status = "移除";
                        }
                    }
                    foreach (var device in newDeviceList)
                    {
                        if (!_DeviceList.Exists(d => d.Path == device.Path))
                        {
                            _DeviceList.Add(new Device() { Path = device.Path, Status = "新增" });
                        }
                    }
                    var removeList = _DeviceList.Where(d => d.Status == "移除").ToList();
                    if (removeList.Count > 0)
                    {
                       Log("移除: " + string.Join(Environment.NewLine, removeList.Select(r => r.Path)));
                        _DeviceList.RemoveAll(r => r.Status == "移除");
                    }
                    var newList = _DeviceList.Where(d => d.Status == "新增").ToList();
                    if (newList.Count > 0)
                    {
                        Log("新增: " + string.Join(Environment.NewLine, newList.Select(r => r.Path)));
                    }
                    _DeviceList.ForEach(d => d.Status = "");
                    //Log("现有: " + string.Join(Environment.NewLine, _DeviceList.Select(r => r.Path)));
                    if (removeList.Count == 0 && newList.Count == 0) Log("无变化");
                    Log("---------");

                }
            }
        }

        private void Log_ui(string s)
        {
            textBox1.BeginInvoke(new Action(() =>
            {
                textBox1.AppendText(s + Environment.NewLine);
            }));
        }

        public List<Device> EnumDevices()
        {
            var devceList = EnumDevicesByGuid(WinApi.Guid_USB);
            devceList.AddRange(EnumDevicesByGuid(WinApi.Guid_HID));
            return devceList;
        }

        public List<Device> EnumDevicesByGuid(Guid guid)
        {
            IntPtr hDevInfo = WinApi.SetupDiGetClassDevs(ref guid, 0, IntPtr.Zero,
                WinApi.DIGCF.DIGCF_PRESENT);
            WinApi.SP_DEVINFO_DATA spDevinfoData = new WinApi.SP_DEVINFO_DATA();

            List<Device> devices = new List<Device>();
            uint n = 0;
            while (true)
            {
                bool result = WinApi.SetupDiEnumDeviceInfo(hDevInfo, n, spDevinfoData);
                //Log(JsonConvert.SerializeObject(spDevinfoData));
                if (!result)
                {
                    break;
                    int lastError = WinApi.GetLastError();
                    if (lastError == 259) break;
                }
                StringBuilder sb = new StringBuilder(3000);
                WinApi.SetupDiGetDeviceRegistryProperty(hDevInfo, spDevinfoData, 
                    1, // SPDRP_HARDWAREID
                    0, 
                    sb, 
                    (uint)sb.Capacity, 
                    IntPtr.Zero);
                //Log(n + ":" + sb.ToString());
                devices.Add(new Device() { Path = sb.ToString(), Status="原有"});
                n++;
            }
            return devices;
        }

        private void ShowCurrentDevices()
        {
            _DeviceList = EnumDevices();
            Log("现有: " + Environment.NewLine + string.Join(Environment.NewLine, _DeviceList.Select(r => r.Path)));
            Log("---------");
        }

        private void button1_Click(object sender, EventArgs e)
        {
            ShowCurrentDevices();
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            ShowCurrentDevices();
            GetAllCerts();
        }

        /// <summary>
        /// 按key值提取a=b,c=d,e=f中相应key的值,如ExtParse("a=b,c=d", "a")返回b，未考虑a=b,a=c的情况
        /// </summary>
        /// <param name="data"></param>
        /// <param name="delimiter"></param>
        /// <returns></returns>
        public string ExtParse(string data, string delimiter)
        {
            string result = String.Empty;
            try
            {
                if (data == null || data == "") return result;

                if (!delimiter.EndsWith("=")) delimiter = delimiter + "=";

                //data = data.ToUpper(); // if you need
                if (!data.Contains(delimiter)) return result;

                int start = data.IndexOf(delimiter) + delimiter.Length;

                string e = data.Substring(start, data.IndexOf('=', start) == -1 ? data.Length - start : data.IndexOf('=', start) - start);
                int tt = e.Length - e.LastIndexOf(", ");

                int length = data.IndexOf('=', start) == -1 ? data.Length - start : data.IndexOf('=', start) - start - tt;

                if (length == 0) return result;
                if (length > 0)
                {
                    result = data.Substring(start, length);

                }
                else
                {
                    result = data.Substring(start);
                }
                return result;

            }
            catch (Exception)
            {
                return result;
            }
        }

    //'2.5.4.3' => 'CN',
    //'2.5.4.4' => 'Surname',
    //'2.5.4.6' => 'C',
    //'2.5.4.7' => 'Cidade',
    //'2.5.4.8' => 'Estado',
    //'2.5.4.9' => 'StreetAddress',
    //'2.5.4.10' => 'O',
    //'2.5.4.11' => 'OU',
    //'2.5.4.12' => 'Title',
    //'2.5.4.20' => 'TelephoneNumber',
    //'2.5.4.42' => 'GivenName',
    //'2.5.29.14' => 'id-ce-subjectKeyIdentifier',
    //'2.5.29.15' => 'id-ce-keyUsage',
    //'2.5.29.17' => 'id-ce-subjectAltName',
    //'2.5.29.19' => 'id-ce-basicConstraints',
    //'2.5.29.20' => 'id-ce-cRLNumber',
    //'2.5.29.31' => 'id-ce-CRLDistributionPoints',
    //'2.5.29.32' => 'id-ce-certificatePolicies',
    //'2.5.29.35' => 'id-ce-authorityKeyIdentifier',
    //'2.5.29.37' => 'id-ce-extKeyUsage',

        /// <summary>
        /// 获取所有certmgr.msc中的“个人”证书相关信息
        /// </summary>
        private List<BankCertificate> GetAllCerts()
        {
            List<BankCertificate> certList = new List<BankCertificate>();
            X509Store store = new X509Store(StoreName.My, StoreLocation.CurrentUser);
            store.Open(OpenFlags.ReadOnly);
            int count = 1;
            foreach (X509Certificate2 certificate in store.Certificates)
            {
                //调winapi: CertNameToStr提取，暂未去实现

                //按oid的方式提取
                var asn = new AsnEncodedData(new Oid { Value = "2.5.4.11" }, certificate.IssuerName.RawData);
                var OU = asn.Format(true);

                //按解析Subject串的方式提取
                var cert = new BankCertificate()
                {
                    Subject = certificate.Subject,
                    NotAfter = certificate.NotAfter,
                    NotBefore = certificate.NotBefore,
                    SerialNumber = certificate.SerialNumber,
                    Thumbprint = certificate.Thumbprint,
                    CN = ExtParse(certificate.Subject, "CN"),
                    O = ExtParse(certificate.Subject, "O"),
                    OU = ExtParse(certificate.Subject, "OU"),
                };

                //var splitComma = certificate.Subject.Split(',');
                //foreach (var span in splitComma)
                //{
                //    var splitEqual = span.Split('=');
                //    if (splitEqual.Length >= 2)
                //    {
                //        var key = splitEqual[0].Trim().ToUpper();
                //        var value = string.Join("", splitEqual.Skip(1)).Trim();
                //        switch (key)
                //        {
                //            case "CN":
                //                cert.CN = value;
                //                break;
                //            case "O":
                //                cert.O = value;
                //                break;
                //            case "OU":
                //                cert.OU = value;
                //                break;
                //        }
                //    }
                //}
                // Log($"{count++}: {JsonConvert.SerializeObject(cert)}");
                certList.Add(cert);
            }
            return certList;
        }

        public class Device
        {
            public string Path { get; set; }
            public string Status { get; set; }
        }

        public class BankCertificate
        {
            public string Subject { get; set; }
            public string CN { get; set; }
            public string O { get; set; }
            public string OU { get; set; }
            public DateTime NotAfter { get; set; }
            public DateTime NotBefore { get;set; }
            public string Thumbprint { get; set; }
            public string SerialNumber { get; set; }
        }
    }
}

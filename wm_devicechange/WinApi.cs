using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace wm_devicechange
{
    public class WinApi
    {
        private const Int32 WM_DEVICECHANGE = 0x219;
        public static Guid Guid_USB = new Guid("36fc9e60-c465-11cf-8056-444553540000");
        public static Guid Guid_HID = new Guid("4d36e96b-e325-11ce-bfc1-08002be10318");

        [DllImport("kernel32", EntryPoint = "GetLastError")]
        public static extern int GetLastError();

        //过滤设备，获取需要的设备
        [DllImport("setupapi.dll", SetLastError = true)]
        public static extern IntPtr SetupDiGetClassDevs(ref Guid ClassGuid, uint Enumerator, IntPtr HwndParent, DIGCF Flags);
        
        [DllImport("setupapi.dll", SetLastError = true)]
        public static extern bool SetupDiEnumDeviceInfo(IntPtr lpInfoSet, UInt32 dwIndex, SP_DEVINFO_DATA devInfoData);

        public struct SP_DEVICE_INTERFACE_DATA
        {
            public int cbSize;
            public Guid interfaceClassGuid;
            public int flags;
            public int reserved;
        }

        [StructLayout(LayoutKind.Sequential)]
        public class SP_DEVINFO_DATA
        {
            public int cbSize = Marshal.SizeOf(typeof(SP_DEVINFO_DATA));
            public Guid classGuid = Guid.Empty; // temp
            public int devInst = 0; // dumy
            public int reserved = 0;
        }

        public enum DIGCF
        {
            DIGCF_DEFAULT = 0x1,
            DIGCF_PRESENT = 0x2,
            DIGCF_ALLCLASSES = 0x4,
            DIGCF_PROFILE = 0x8,
            DIGCF_DEVICEINTERFACE = 0x10
        }

        [DllImport("setupapi.dll", SetLastError = true)]
        public static extern bool SetupDiGetDeviceRegistryProperty(
            IntPtr lpInfoSet, 
            SP_DEVINFO_DATA DeviceInfoData, 
            UInt32 Property, 
            UInt32 PropertyRegDataType, 
            StringBuilder PropertyBuffer, 
            UInt32 PropertyBufferSize, 
            IntPtr RequiredSize);


        private static readonly Int32 X509_ASN_ENCODING = 1;

        private static readonly Int32 CERT_SIMPLE_NAME_STR = 1;
        private static readonly Int32 CERT_OID_NAME_STR = 2;
        private static readonly Int32 CERT_NAME_STR_CRLF_FLAG = 0x08000000;
        private static readonly Int32 CERT_NAME_STR_NO_QUOTING_FLAG = 0x10000000;
        private static readonly Int32 CERT_NAME_STR_REVERSE_FLAG = 0x02000000;
        
        [StructLayout(LayoutKind.Sequential)]
        public struct CERT_NAME_BLOB
        {
            public Int32 cbData;
            public IntPtr pbData;
        }
        [DllImport("CRYPT32.DLL", EntryPoint = "CertNameToStr", CharSet = CharSet.Auto, SetLastError = true)]

        public static extern Int32 CertNameToStr(

                Int32 dwCertEncodingType,

                ref CERT_NAME_BLOB pName,

                Int32 dwStrType,

                StringBuilder psz,

                Int32 csz

            );

    }
}

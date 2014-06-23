using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DarkFriendsInjector
{
    public class HookInject
    {
        public static bool Hook()
        {
            string dllPath = Assembly.GetExecutingAssembly().Location;
            dllPath = Path.Combine(Path.GetDirectoryName(dllPath), "DarkFriends.dll");
            if(!File.Exists(dllPath))
            {
                throw new Exception("DarkFriends.dll not found");
            }

            IntPtr window = Native.FindWindow(null, "DARK SOULS II");
            if(window.ToInt32() == 0)
            {
                return false;
            }

            uint pid;
            Native.GetWindowThreadProcessId(window, out pid);
            if(pid == 0)
            {
                throw new Exception("Failed to get pid for window");
            }

            IntPtr processHandle = Native.OpenProcess(Native.ProcessAccessFlags.All, false, pid);
            if(processHandle.ToInt32() == 0)
            {
                throw new Exception("Failed to get handle for pid");
            }

            IntPtr loadLibraryAddr = Native.GetProcAddress(Native.GetModuleHandle("kernel32.dll"), "LoadLibraryA");
            if(loadLibraryAddr.ToInt32() == 0)
            {
                throw new Exception("Failed to get LoadLibraryA address");
            }

            IntPtr allocated = Native.VirtualAllocEx(processHandle, IntPtr.Zero, (uint)dllPath.Length, Native.AllocationType.Reserve | Native.AllocationType.Commit, Native.MemoryProtection.ReadWrite);
            if(allocated.ToInt32() == 0)
            {
                throw new Exception("Failed to allocate memory in remote thread");
            }

            IntPtr bytesWritten;
            if(!Native.WriteProcessMemory(processHandle, allocated, Encoding.UTF8.GetBytes(dllPath), dllPath.Length, out bytesWritten))
            {
                throw new Exception("Failed to write memory to process");
            }

            IntPtr threadId = Native.CreateRemoteThread(processHandle, IntPtr.Zero, 0, loadLibraryAddr, allocated, 0, IntPtr.Zero);
            if(threadId.ToInt32() == 0)
            {
                throw new Exception("Failed to create new thread");
            }

            Native.CloseHandle(processHandle);

            return true;
        }

        public static bool Unhook()
        {
            throw new NotImplementedException();
        }
    }
}

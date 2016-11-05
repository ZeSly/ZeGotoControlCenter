using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using ASCOM.Utilities;
using ASCOM.Utilities.Interfaces;
using System.Runtime.InteropServices;

namespace ASCOM.ZeGoto
{
    [ComVisible(false)]
    public class SerialOrSocket : ISerial, IDisposable
    {
        private static Serial serial;

        public SerialOrSocket()
        {
            serial = new Serial();
        }

        private bool m_TcpIp;
        private Socket socket;

        public bool TcpIp
        {
            get { return m_TcpIp; }
            set { if (!Connected) m_TcpIp = value; }
        }

        public string[] AvailableComPorts { get { return serial.AvailableCOMPorts; } }

        private bool m_Connected;

        public bool Connected
        {
            get { return TcpIp ? m_Connected : serial.Connected; }
            set
            {
                if (TcpIp)
                {
                    if (value)
                    {
                        if (!m_Connected)
                        {
                            socket = null;
                            IPHostEntry hostEntry = null;

                            hostEntry = Dns.GetHostEntry(IPAddress);

                            // Loop through the AddressList to obtain the supported AddressFamily. This is to avoid
                            // an exception that occurs when the host IP Address is not compatible with the address family
                            // (typical in the IPv6 case).
                            foreach (IPAddress address in hostEntry.AddressList)
                            {
                                IPEndPoint ipe = new IPEndPoint(address, m_Port);
                                Socket tempSocket = new Socket(ipe.AddressFamily, SocketType.Stream, ProtocolType.Tcp);

                                tempSocket.Connect(ipe);

                                if (tempSocket.Connected)
                                {
                                    socket = tempSocket;
                                    break;
                                }
                                else
                                {
                                    continue;
                                }
                            }

                            if (socket != null)
                            {
                                if (m_ReceiveTimeout != 0) socket.ReceiveTimeout = m_ReceiveTimeout;
                                m_Connected = true;
                            }
                        }
                    }
                    else
                    {
                        socket.Shutdown(SocketShutdown.Both);
                        socket.Disconnect(false);
                        socket = null;
                        m_Connected = value;
                    }
                }
                else
                {
                    serial.Connected = value;
                }
            }
        }

        public int DataBits { get { return serial.DataBits; } set { serial.DataBits = value; } }
        public bool DTREnable { get { return serial.DTREnable; } set { serial.DTREnable = value; } }
        public SerialHandshake Handshake { get { return serial.Handshake; } set { serial.Handshake = value; } }
        public SerialParity Parity { get { return serial.Parity; } set { serial.Parity = value; } }

        private int m_Port;

        public int Port
        {
            get { return m_Port; }
            set
            {
                if (!TcpIp) serial.Port = value;
                m_Port = value;
            }
        }

        public string PortName { get; set; }

        private int m_ReceiveTimeout = 0;
        public int ReceiveTimeout
        {
            get { return TcpIp ? m_ReceiveTimeout / 1000 : serial.ReceiveTimeout; }
            set
            {
                serial.ReceiveTimeout = value;
                m_ReceiveTimeout = value * 1000;
                if (TcpIp && m_Connected)
                {
                    socket.ReceiveTimeout = m_ReceiveTimeout;
                }
            }
        }

        public int ReceiveTimeoutMs
        {
            get { return TcpIp ? m_ReceiveTimeout : serial.ReceiveTimeout; }
            set
            {
                serial.ReceiveTimeoutMs = value;
                m_ReceiveTimeout = value;
                if (TcpIp && m_Connected)
                {
                    socket.ReceiveTimeout = m_ReceiveTimeout;
                }
            }
        }
        public bool RTSEnable { get { return serial.RTSEnable; } set { serial.RTSEnable = value; } }
        public SerialSpeed Speed { get { return serial.Speed; } set { serial.Speed = value; } }
        public SerialStopBits StopBits { get { return serial.StopBits; } set { serial.StopBits = value; } }

        public string IPAddress;

        public void ClearBuffers()
        {
            if (!TcpIp) serial.ClearBuffers();
        }

        public void Dispose()
        {
            serial.Dispose();
        }

        //protected virtual void Dispose(bool disposing);
        public void LogMessage(string Caller, string Message)
        {
            serial.LogMessage(Caller, Message);
        }

        public string Receive()
        {
            if (!TcpIp) return serial.Receive();

            byte[] buffer;
            if (socket.Available > 0) buffer = new byte[socket.Available];
            else buffer = new byte[128];

            int n = socket.Receive(buffer);
            return Encoding.ASCII.GetString(buffer, 0, n);
        }

        public byte ReceiveByte()
        {
            if (!TcpIp) return serial.ReceiveByte();

            byte[] buffer = new byte[1];
            socket.Receive(buffer, 1, 0);
            return buffer[0];
        }

        public string ReceiveCounted(int Count)
        {
            if (!TcpIp) return serial.ReceiveCounted(Count);

            byte[] buffer = new byte[Count];

            int n = socket.Receive(buffer, Count, 0);
            return Encoding.ASCII.GetString(buffer, 0, n);
        }

        public byte[] ReceiveCountedBinary(int Count)
        {
            if (!TcpIp) return serial.ReceiveCountedBinary(Count);

            byte[] buffer = new byte[Count];
            socket.Receive(buffer, Count, 0);
            return buffer;
        }

        public string ReceiveTerminated(string Terminator)
        {
            if (!TcpIp) return serial.ReceiveTerminated(Terminator);

            string s = "";
            do
            {
                byte[] buffer;
                if (socket.Available > 0) buffer = new byte[socket.Available];
                else buffer = new byte[128];

                int n = socket.Receive(buffer);
                s += Encoding.ASCII.GetString(buffer, 0, n);
            }
            while (!s.EndsWith(Terminator));

            return s;
        }

        public byte[] ReceiveTerminatedBinary(byte[] TerminatorBytes)
        {
            if (!TcpIp) return serial.ReceiveTerminatedBinary(TerminatorBytes);

            return null;
        }

        public void Transmit(string Data)
        {
            if (!TcpIp)
            {
                serial.Transmit(Data);
            }
            else
            {
                socket.Send(Encoding.ASCII.GetBytes(Data));
            }
        }

        public void TransmitBinary(byte[] Data)
        {
            if (!TcpIp)
            {
                serial.TransmitBinary(Data);
            }
            else
            {
                socket.Send(Data);
            }
        }
    }
}

package com.example.emvl3app;

import android.util.Log;

import com.szzt.android.util.HexDump;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import com.szzt.android.util.HexDump;

public class TCPClient {
    private Socket socket;
    private InputStream inputStream;
    private OutputStream outputStream;
    private String ipAddress;
    private int port;

    public TCPClient() {
        socket = null;
        inputStream = null;
        outputStream = null;
    }

    public int getPort(){
        return port;
    }

    public String getIpAddress(){
        return ipAddress;
    }

    public void connect(String ipAddress, int port, int bufferSize) throws IOException {
        Log.d("lishiyao", "start TCP connect");
        Log.d("lishiyao", "input bufferSize:"+bufferSize);
        socket = new Socket(ipAddress, port);
        socket.setReceiveBufferSize(bufferSize);
        socket.setSendBufferSize(bufferSize);
        inputStream = socket.getInputStream();
        outputStream = socket.getOutputStream();
        Log.d("lishiyao", "TCP connection established");
    }

    public void send(byte[] data) throws IOException {
        if (outputStream != null) {
            outputStream.write(data);
            outputStream.flush();
            Log.d("lishiyao","Data sent: " + HexDump.dumpHex(data));
        }
    }

    public int read(byte[] buffer) throws IOException {
        byte[] header = new byte[4];
        byte[] tmpBuf = new byte[512];
        int tlvLen, lenAll=0;
        int index;

        if (inputStream != null)
        {
            int bytesRead = inputStream.read(header, 0, 4);
            Log.d("lishiyao","act read TCP buffer len:"+bytesRead);
            if (bytesRead == 4)
            {
                Log.d("lishiyao","receive header: " + HexDump.dumpHex(header));
                byte[] tmplen = new byte[2];
                if(header[0] == 0x02)
                {
                    tmplen[0] = header[2];
                    tmplen[1] = header[3];
                    tlvLen = HexDump.byteToInt(tmplen);
                    Log.d("lishiyao","receive TLV len:"+tlvLen);
                    System.arraycopy(header, 0, buffer, 0, 4);
                    index = 4;
                    lenAll = 0;
                    while (lenAll != tlvLen)
                    {
                        bytesRead = inputStream.read(buffer, index, tlvLen-lenAll);

                        if(bytesRead == -1)
                        {
                            Log.d("lishiyao", "break flag1 ");
                            break;
                        }
//                        inputStream.skip(bytesRead);
                        lenAll += bytesRead;
                        index += bytesRead;
                        if(lenAll > tlvLen)
                        {
                            Log.d("lishiyao", "break flag2 ");
                            break;
                        }
                    }

                }
                else
                {
                    Log.d("lishiyao","invalid header");
                    return TypeDefine.EMV_PARAM_INVALID;
                }

                Log.d("lishiyao","Data receive: " + HexDump.dumpHex(buffer));
                Log.d("lishiyao","Data receive len: " + buffer.length+":"+inputStream.available());
                if(inputStream.available() > 0)
                {
                    byte []temp = new byte[inputStream.available()];
                    inputStream.read(temp);
                }
                return lenAll+4;
            }
            else
            {
                Log.d("lishiyao","read no tcp data");
                return TypeDefine.EMV_ERR;
            }
        }
        Log.d("lishiyao","inputStream is null");
        return TypeDefine.EMV_PARAM_INVALID;
    }

    public void close() throws IOException {
        if (socket != null) {
            socket.close();
            Log.d("lishiyao","TCP connection closed");
        }
    }

    public boolean getStatus(){
        if(socket.isClosed())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
}

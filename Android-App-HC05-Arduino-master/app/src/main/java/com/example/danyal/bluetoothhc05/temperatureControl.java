package com.example.danyal.bluetoothhc05;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.lang.ref.WeakReference;
import java.util.UUID;

public class TemperatureControl extends AppCompatActivity {

    Button btnEnviar, btnDis;
    EditText editTextT;
    String address = null;
    TextView textError,curTemp;
    Thermometer thermometer;
    ConnectedThread cThread;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
    Float tAct;
    private final MyHandler mHandler = new MyHandler(this);
    private boolean isBtConnected = false;
    static final UUID myUUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS);
        setContentView(R.layout.temperature_control);
        btnEnviar = (Button) findViewById(R.id.buttonEnviar);
        btnDis = (Button) findViewById(R.id.button4);
        textError= (TextView) findViewById(R.id.textError);
        editTextT = (EditText) findViewById(R.id.editTextT);
        editTextT.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                String texto= editable.toString();
                try{
                    Float f= Float.parseFloat(texto);
                    if(f>25.0 && f<50.0 && isBtConnected){
                            btnEnviar.setEnabled(true);
                            textError.setVisibility(View.GONE);
                        }
                    else{
                        btnEnviar.setEnabled(false);
                        textError.setVisibility(View.VISIBLE);
                    }
                }
                catch (Exception ignored){

                }
            }
        });
        curTemp= (TextView) findViewById(R.id.curTemp);
        thermometer= (Thermometer) findViewById(R.id.thermometer);
        thermometer.changeInnerColor(Color.RED);
        new ConnectBT().execute();

        btnEnviar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick (View v) {
                cThread.write("T"+editTextT.getText().toString());
                btnDis.setVisibility(View.VISIBLE);
                btnEnviar.setVisibility(View.GONE);
            }
        });

        btnDis.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick (View v) {
                cThread.write("C");
                Disconnect();
            }
        });
    }

    private static class MyHandler extends Handler {
        private final WeakReference<TemperatureControl> mActivity;

        public MyHandler(TemperatureControl activity) {
            mActivity = new WeakReference<TemperatureControl>(activity);
        }

        @Override
        public void handleMessage(Message msg) {
            TemperatureControl activity = mActivity.get();
            if (activity != null) {
                String readMessage = (String) msg.obj;
                if(readMessage.startsWith("H")){
                    String[] params= readMessage.split(":::");
                    activity.thermometer.changeInnerColor(Color.RED);
                    activity.setTAct(params[1]);
                }
                else if(readMessage.startsWith("F")){
                    activity.msg("Temperatura fuera de rango");
                }
                else if(readMessage.startsWith("V")){
                    String[] params= readMessage.split(":::");
                    activity.thermometer.changeInnerColor(Color.BLUE);
                    activity.setTAct(params[1]);
                }
            }
        }
    }

    private String receiveSignal(){
        if ( btSocket != null ) {
            try {
                byte[] buffer = new byte[1024];
                int bytes= btSocket.getInputStream().read(buffer);
                return new String(buffer, 0, bytes);
            } catch (IOException e) {
                msg("Error");
            }
        }
        return null;
    }

    private void Disconnect () {
        if ( btSocket!=null ) {
            try {
                btSocket.close();
            } catch(IOException e) {
                msg("Error");
            }
        }

        finish();
    }

    private void setTAct(String param){
        tAct= Float.parseFloat(param);
        curTemp.setText(param);
        thermometer.setCurrentTemp(tAct);
    }

    private void msg (String s) {
        Toast.makeText(getApplicationContext(), s, Toast.LENGTH_LONG).show();
    }

    private class ConnectBT extends AsyncTask<Void, Void, Void> {
        private boolean ConnectSuccess = true;

        @Override
        protected Void doInBackground (Void... devices) {
            try {
                if ( btSocket==null || !isBtConnected ) {
                    myBluetooth = BluetoothAdapter.getDefaultAdapter();
                    BluetoothDevice dispositivo = myBluetooth.getRemoteDevice(address);
                    btSocket = dispositivo.createInsecureRfcommSocketToServiceRecord(myUUID);
                    BluetoothAdapter.getDefaultAdapter().cancelDiscovery();
                    btSocket.connect();
                }
            } catch (IOException e) {
                ConnectSuccess = false;
            }

            return null;
        }

        @Override
        protected void onPostExecute (Void result) {
            super.onPostExecute(result);

            if (!ConnectSuccess) {
                msg("Connection Failed. Is it a SPP Bluetooth? Try again.");
                finish();
            } else {
                msg("Connected");
                cThread= new ConnectedThread(btSocket);
                isBtConnected = true;
            }
        }
    }

    //create new class for connect thread
    private class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private PrintWriter mmOutStream;

        //creation of the connect thread
        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                //Create I/O streams for connection
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException ignored) { }
            mmInStream = tmpIn;
            if(tmpOut!=null){
                mmOutStream = new PrintWriter(tmpOut);
            }

        }

        public void run() {
            byte[] buffer = new byte[256];
            int bytes;

            // Keep looping to listen for received messages
            while (true) {
                try {
                    bytes = mmInStream.read(buffer);            //read bytes from input buffer
                    String readMessage = new String(buffer, 0, bytes);
                    // Send the obtained bytes to the UI Activity via handler
                    mHandler.obtainMessage(0, bytes, -1, readMessage).sendToTarget();
                } catch (IOException e) {
                    break;
                }
            }
        }
        //write method
        public void write(String input) {          //converts entered String into bytes
            mmOutStream.println(input);                //write bytes over BT connection via outstream
            mmOutStream.flush();
        }
    }
}

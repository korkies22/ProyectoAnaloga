package com.example.danyal.bluetoothhc05;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.AsyncTask;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.UUID;

public class temperatureControl extends AppCompatActivity {

    Button btnEnviar, btnDis;
    EditText editTextT;
    String address = null;
    TextView textError,curTemp;
    Thermometer thermometer;
    private ProgressDialog progress;
    BluetoothAdapter myBluetooth = null;
    BluetoothSocket btSocket = null;
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
        btnDis = (Button) findViewById(R.id.button4);
        editTextT = (EditText) findViewById(R.id.editTextT);
        textError= (TextView) findViewById(R.id.textError);
        curTemp= (TextView) findViewById(R.id.curTemp);
        thermometer= (Thermometer) findViewById(R.id.thermometer);

        new ConnectBT().execute();

        btnEnviar.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick (View v) {
                sendSignal("T");
                String message= receiveSignal();
                if(message!=null){
                    float num= Float.parseFloat(message);
                    curTemp.setText(message);
                    thermometer.setCurrentTemp(num);
                }
                sendSignal(editTextT.getText().toString());
            }
        });

        btnDis.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick (View v) {
                sendSignal("C");
                Disconnect();
            }
        });
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

    private void sendSignal ( String number ) {
        if ( btSocket != null ) {
            try {
                btSocket.getOutputStream().write(number.toString().getBytes());
            } catch (IOException e) {
                msg("Error");
            }
        }
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
                isBtConnected = true;
            }

            progress.dismiss();
        }
    }
}

package com.example.jzm.ttrs;

import android.content.Intent;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.view.WindowManager;
import android.widget.*;

import org.json.JSONObject;

import java.io.UnsupportedEncodingException;

public class Login extends AppCompatActivity implements View.OnClickListener {

    private EditText userId;
    private EditText password;
    private Button buttonLogin;
    private Button buttonRegister;
    private CheckBox rememberPassword;
    private SharedPreferences pref;
    private SharedPreferences.Editor editor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);
        init();
        buttonLogin.setOnClickListener(this);
        buttonRegister.setOnClickListener(this);
        pref = PreferenceManager.getDefaultSharedPreferences(this);
        rememberPassword = findViewById(R.id.login_Checkbox);
        boolean isRemember = pref.getBoolean("remember_password", false);
        if (isRemember){
            userId.setText(pref.getString("id", ""));
            password.setText(pref.getString("password", ""));
            rememberPassword.setChecked(true);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data){
        switch (requestCode){
            case 1:
                if (resultCode == RESULT_OK){
                    String id = data.getStringExtra("id");
                    userId.setText(id);
                    showResponse("注册成功！这是您的用户ID，请记住它~♪（＾∀＾●）");
                }
                break;
            default:
                break;
        }
    }

    private void init(){
        buttonLogin = findViewById(R.id.login_LoginButton);
        buttonRegister = findViewById(R.id.login_RegisterButton);
        userId = findViewById(R.id.loginUserid_Edit);
        password = findViewById(R.id.loginPassword_Edit);
        rememberPassword = findViewById(R.id.login_Checkbox);
        ImageView unameClear = findViewById(R.id.loginUserid_Clear);
        ImageView pwdClear = findViewById(R.id.loginPassword_Clear);
        EditTextClearTools.addClearListener(userId,unameClear);
        EditTextClearTools.addClearListener(password,pwdClear);
    }

    public void onClick(View view){
        switch (view.getId()){
            case R.id.login_LoginButton:
                String suserid = userId.getText().toString();
                String spassword = password.getText().toString();
                try {
                    if (!usernameCheck(suserid)) break;
                    if (!passwordCheck(spassword)) break;
                }catch (Exception e){
                    e.printStackTrace();
                }
                sendRequest();
                break;
            case R.id.login_RegisterButton:{
                Intent intent = new Intent(Login.this, Register.class);
                startActivityForResult(intent, 1);
            }
            default:
                break;
        }
    }

    private void sendRequest(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                try{
                    HttpClient client = new HttpClient();
                    String suserid = userId.getText().toString();
                    String spassword = password.getText().toString();
                    client.setCommand("{\"type\":\"login\",\"id\":\""+suserid+"\",\"password\":\""+spassword+"\"}");
                    JSONObject jsonObject = new JSONObject(client.run());
                    String success = jsonObject.getString("success");
                    if (success == "true"){
                        editor = pref.edit();
                        if (rememberPassword.isChecked()){
                            editor.putBoolean("remember_password", true);
                            editor.putString("id", suserid);
                            editor.putString("password", spassword);
                        }else{
                            editor.clear();
                        }
                        editor.apply();
                        Intent intent = new Intent(Login.this, MainActivity.class);
                        JSONObjectStringCreate jsonObjectStringCreate = new JSONObjectStringCreate();
                        jsonObjectStringCreate.addStringPair("type", "query_profile");
                        jsonObjectStringCreate.addStringPair("id", suserid);
                        client.setCommand(jsonObjectStringCreate.getResult());
                        JSONObject result = new JSONObject(client.run());
                        result.put("id", suserid);
                        result.put("password", spassword);
                        intent.putExtra("info", result.toString());
                        startActivity(intent);
                        finish();
                    }else{
                        showResponse("用户名密码不符~QAQ~");
                    }
                }catch (Exception e){
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void showResponse(final String message) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                password.setText("");
                Toast.makeText(Login.this, message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    private boolean empty(String s, String message){
        if (s.equals("")) {
            showResponse("未输入" + message + "呀~QAQ~");
            return true;
        }else return false;
    }

    private boolean tooLong(String s, String message) throws UnsupportedEncodingException {
        if (s.getBytes("UTF-8").length > 20){
            showResponse(message + "太长了呀~QAQ");
            return true;
        }else return false;
    }

    private boolean checkWhiteSpace(String s, String message){
        if (s.contains(" ")) {
            showResponse(message + "不能有空格呀~QAQ~");
            return true;
        }else return false;
    }

    private boolean usernameCheck(String s) throws UnsupportedEncodingException {
        if (empty(s, "id")) return false;
        if (tooLong(s, "id")) return false;
        if (checkWhiteSpace(s, "id")) return false;
        return true;
    }

    private boolean passwordCheck(String s) throws UnsupportedEncodingException {
        if (empty(s, "密码")) return false;
        if (tooLong(s, "密码")) return false;
        if (checkWhiteSpace(s, "密码")) return false;
        return true;
    }
}

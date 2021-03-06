package com.example.jzm.ttrsadmin;

import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import org.json.JSONObject;

import java.io.UnsupportedEncodingException;

import es.dmoral.toasty.Toasty;

public class ContentFragment_train_detail extends Fragment {
    private View view;

    ProgressbarFragment progressbarFragment = new ProgressbarFragment();

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState){
        view = inflater.inflate(R.layout.contain_trainid_query, container, false);
        final EditText editText = view.findViewById(R.id.trainid_Edit);
        Button button = view.findViewById(R.id.contain_trainid_query_button);
        button.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View view) {
                String trainId = editText.getText().toString();
                try {
                    if (!trainIdCheck(trainId)){
                        return;
                    }else {
                        progressbarFragment.setCancelable(false);
                        progressbarFragment.show(getActivity().getFragmentManager());
                        sendRequest();
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        return view;
    }

    private void sendRequest(){
        new Thread(new Runnable() {
            @Override
            public void run() {
                try{
                    HttpClient client = new HttpClient();
                    EditText editText = view.findViewById(R.id.trainid_Edit);
                    String trainId = editText.getText().toString();
                    client.setCommand("{\"type\":\"query_train\",\"train_id\":\""+trainId+"\"}");
                    String response = client.run();
                    JSONObject jsonObject = new JSONObject(response);
                    String success = jsonObject.getString("success");
                    if (success.equals("true")){
                        Intent intent = new Intent(getActivity(), TrainDetailManifest.class);
                        intent.putExtra("JSON", jsonObject.toString());
                        progressbarFragment.dismiss();
                        startActivity(intent);
                    }else{
                        progressbarFragment.dismiss();
                        showResponse("这是一辆幽灵列车", "error");
                    }
                }catch (Exception e){
                    showResponse("小熊猫联系不上饲养员了，请检查网络连接%>_<%", "warning");
                    try{
                        progressbarFragment.dismiss();
                    }catch (Exception ex){
                        ex.printStackTrace();
                    }
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void showResponse(final String message, final String type){
        getActivity().runOnUiThread(new Runnable() {
            @Override
            public void run() {
                try {
                    switch (type) {
                        case "error": {
                            Toasty.error(getActivity(), message, Toast.LENGTH_SHORT).show();
                            break;
                        }
                        case "success": {
                            Toasty.success(getActivity(), message, Toast.LENGTH_SHORT).show();
                            break;
                        }
                        case "info": {
                            Toasty.info(getActivity(), message, Toast.LENGTH_SHORT).show();
                            break;
                        }
                        case "warning": {
                            Toasty.warning(getActivity(), message, Toast.LENGTH_SHORT).show();
                            break;
                        }
                    }
                }catch (Exception e){
                    e.printStackTrace();
                }
            }
        });
    }
    
    private boolean empty(String s, String message){
        if (s.equals("")) {
            showResponse("未输入" + message + "呀~QAQ~", "info");
            return true;
        }else return false;
    }

    private boolean tooLong(String s, String message) throws UnsupportedEncodingException {
        int maxLength = 20;
        if (message.equals("用户名")) maxLength = 40;
        if (s.getBytes("UTF-8").length > maxLength){
            showResponse(message + "太长了呀~QAQ", "info");
            return true;
        }else return false;
    }

    private boolean checkWhiteSpace(String s, String message){
        if (s.contains(" ")) {
            showResponse(message + "不能有空格呀~QAQ~", "info");
            return true;
        }else return false;
    }

    private boolean trainIdCheck(String s) throws UnsupportedEncodingException {
        if (empty(s, "列车的ID")) return false;
        if (tooLong(s, "列车的ID")) return false;
        if (checkWhiteSpace(s, "列车的ID")) return false;
        return true;
    }

}

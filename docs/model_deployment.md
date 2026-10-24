# Deploying the Detection Model

The Grove Vision AI V2 runs the object detection model used by the tracker. This guide explains how to deploy a model to the module with SenseCraft AI, Seeed Studio's web-based platform for compatible edge-AI devices. No firmware needs to be built or flashed manually.

Two workflows are available: the main SenseCraft AI model library, and the SenseCraft AI Model Assistant. Use either one.

## Before You Start

- Connect the Grove Vision AI V2 to your computer with a USB cable.
- Use a browser that supports Web Serial (for example Chrome or Edge). Both workflows ask the browser to select the module's serial port.

## Option 1: SenseCraft AI (Model Library)

Select a model from the SenseCraft AI library and deploy it to the module.

1. Open [SenseCraft AI](https://sensecraft.seeed.cc/ai/model/).

2. Select the required model.

   ![Model selection](assets/images/SenseCraft_AI/choose-model.png)

3. Click **Deploy Model**.

   ![Deploy Model button](assets/images/SenseCraft_AI/deploy-model.png)

4. Scroll down and click **Connect Device**.

   ![Connect Device button](assets/images/SenseCraft_AI/connect.png)

5. In the dialog, click **Confirm**.

   ![Confirmation dialog](assets/images/SenseCraft_AI/confirm-deployment.png)

6. In the browser's serial-port prompt, select the port connected to the Grove Vision AI V2 and click **Connect**.

   ![Port selection prompt](assets/images/SenseCraft_AI/com-port-selection.png)

7. Wait until the model has been deployed and flashed to the module.

8. Test the result with the camera **Preview**. The **Device Logger** shows inference information from the module.

   ![Preview and Device Logger](assets/images/SenseCraft_AI/preview-device-logger.png)

9. For detection models, adjust the **Confidence** and **IoU** thresholds with their sliders.

   ![Confidence and IoU sliders](assets/images/SenseCraft_AI/preview-settings.png)

## Option 2: SenseCraft AI Model Assistant

The SenseCraft AI Model Assistant is another web-based workflow for connecting to the module and deploying a selected model.

1. Open the [SenseCraft AI Model Assistant](https://seeed-studio.github.io/SenseCraft-Web-Toolkit/#/setup/process).

2. In the upper-right corner, open the device menu, choose **Grove Vision AI (V2)** and click **Connect**.

   ![Device selection menu](assets/images/SenseCraft_AI_Model_Assistant/choose-board-connect.png)

3. In the browser's serial-port prompt, select the port connected to the Grove Vision AI V2 and click **Connect**.

   ![Port selection prompt](assets/images/SenseCraft_AI_Model_Assistant/com-port-selection.png)

4. Select the required model and click **Send**.

   ![Model selection and Send button](assets/images/SenseCraft_AI_Model_Assistant/select-model-send.png)

5. Wait until the model has been deployed and flashed to the module.

6. Test the inference results with the camera **Preview**.

   ![Camera Preview](assets/images/SenseCraft_AI_Model_Assistant/preview.png)

7. Turn the **Device Logger** on or off with the document-shaped logger button.

   ![Device Logger button](assets/images/SenseCraft_AI_Model_Assistant/device-logger.png)

8. For detection models, adjust the **Confidence** and **IoU** thresholds with their sliders.

   ![Confidence and IoU sliders](assets/images/SenseCraft_AI_Model_Assistant/preview-settings.png)

## References

- [SenseCraft AI](https://sensecraft.seeed.cc/ai/model/)
- [SenseCraft AI Model Assistant](https://seeed-studio.github.io/SenseCraft-Web-Toolkit/#/setup/process)
- [Grove Vision AI V2 — Software Support](https://wiki.seeedstudio.com/grove_vision_ai_v2_software_support/)
- [Grove Vision AI V2a](https://wiki.seeedstudio.com/grove_vision_ai_v2a/)
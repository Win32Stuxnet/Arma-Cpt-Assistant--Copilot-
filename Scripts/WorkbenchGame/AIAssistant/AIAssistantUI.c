//-----------------------------------------------------------------------------
//! User Interface for AI Assistant plugin
//! Provides Workbench dialogs for interacting with the Copilot bridge
//-----------------------------------------------------------------------------

class AIAssistantPlugin;

class AIAssistantUI
{
        protected ref AIAssistantCore m_AICore;
        protected AIAssistantPlugin m_Plugin;
        protected bool m_IsMainDialogOpen;
        protected bool m_IsSettingsDialogOpen;
        protected ref array<string> m_RequestTypeLabels;
        protected ref array<AIRequestType> m_RequestTypeValues;

        //-----------------------------------------------------------------------------
        void AIAssistantUI(AIAssistantCore aiCore, AIAssistantPlugin plugin)
        {
                m_AICore = aiCore;
                m_Plugin = plugin;
                m_IsMainDialogOpen = false;
                m_IsSettingsDialogOpen = false;

                InitialiseRequestTypes();
        }

        //-----------------------------------------------------------------------------
        //! Show main AI Assistant dialog
        void ShowMainDialog(WorkbenchContext context)
        {
                if (m_IsMainDialogOpen)
                        return;

                m_IsMainDialogOpen = true;
                ShowRequestDialog(context);
        }

        //-----------------------------------------------------------------------------
        //! Show settings dialog
        void ShowSettingsDialog()
        {
                if (m_IsSettingsDialogOpen)
                        return;

                m_IsSettingsDialogOpen = true;
                ShowSettingsDialogInternal();
        }

        //-----------------------------------------------------------------------------
        //! Build and present the request dialog
protected void ShowRequestDialog(WorkbenchContext context)
{
ref array<ref ScriptDialogInputBase> inputs = {};

AIAssistantSettings settings = m_AICore.GetSettings();

ScriptDialogInputCombo typeInput = new ScriptDialogInputCombo("Task", m_RequestTypeLabels, 0);
inputs.Insert(typeInput);

ScriptDialogInputText promptInput = new ScriptDialogInputText("Prompt", "");
inputs.Insert(promptInput);

ScriptDialogInputCheckBox includeSelectionInput = new ScriptDialogInputCheckBox("Include selected script/code", true);
inputs.Insert(includeSelectionInput);

ScriptDialogInputCheckBox insertIntoEditorInput = new ScriptDialogInputCheckBox("Insert generated code into editor", settings.GetAutoInsertCode());
inputs.Insert(insertIntoEditorInput);

bool confirmed = Workbench.ScriptDialog().Show("AI Copilot", "Send", "Cancel", inputs);
m_IsMainDialogOpen = false;

if (!confirmed)
return;

string userPrompt = promptInput.GetValue().Trim();
if (userPrompt.IsEmpty())
{
ShowMessage("Please enter a request for the AI assistant.");
return;
}

                AIRequestType requestType = m_RequestTypeValues[typeInput.GetValue()];

                if (includeSelectionInput.GetValue())
                        userPrompt = AppendSelectionContext(userPrompt, context);

if (insertIntoEditorInput.GetValue() != settings.GetAutoInsertCode())
{
settings.SetAutoInsertCode(insertIntoEditorInput.GetValue());
}

                ProcessAIRequest(requestType, userPrompt);
        }

        //-----------------------------------------------------------------------------
        //! Build request type options shown in the dialog
protected void InitialiseRequestTypes()
{
m_RequestTypeLabels = {"General chat", "Generate code", "Analyse code", "Debug code", "Document code", "Optimise code", "Explain code", "Refactor code"};
m_RequestTypeValues = {AIRequestType.GENERAL_CHAT, AIRequestType.CODE_GENERATION, AIRequestType.CODE_ANALYSIS, AIRequestType.CODE_DEBUGGING, AIRequestType.DOCUMENTATION, AIRequestType.OPTIMIZATION, AIRequestType.EXPLANATION, AIRequestType.REFACTORING};
}

        //-----------------------------------------------------------------------------
        //! Append current selection information to the user prompt
        protected string AppendSelectionContext(string prompt, WorkbenchContext context)
        {
                string selection = m_AICore.GetSelectedCode(context);
                if (selection.IsEmpty())
                        return prompt;

                string enrichedPrompt = prompt;
                enrichedPrompt += "\n\n=== Selected Script Context ===\n";
                enrichedPrompt += selection;

                return enrichedPrompt;
        }

        //-----------------------------------------------------------------------------
        //! Create a simple history preview to show inside the dialog
//-----------------------------------------------------------------------------
//! Display Workbench settings dialog allowing configuration changes
protected void ShowSettingsDialogInternal()
{
AIAssistantSettings settings = m_AICore.GetSettings();

ref array<ref ScriptDialogInputBase> inputs = {};

ref array<string> serviceLabels = {"Claude API", "OpenAI API", "Local model", "Custom endpoint"};
int serviceIndex = settings.GetServiceProvider();
ScriptDialogInputCombo serviceInput = new ScriptDialogInputCombo("Service provider", serviceLabels, serviceIndex);
inputs.Insert(serviceInput);

ScriptDialogInputText apiKeyInput = new ScriptDialogInputText("API key", settings.GetAPIKey());
inputs.Insert(apiKeyInput);

ScriptDialogInputText modelInput = new ScriptDialogInputText("Model name", settings.GetModelName());
inputs.Insert(modelInput);

ScriptDialogInputText endpointInput = new ScriptDialogInputText("Custom endpoint", settings.GetCustomEndpoint());
inputs.Insert(endpointInput);

ScriptDialogInputText temperatureInput = new ScriptDialogInputText("Temperature", settings.GetTemperature().ToString());
inputs.Insert(temperatureInput);

ScriptDialogInputText maxTokensInput = new ScriptDialogInputText("Max tokens", settings.GetMaxTokens().ToString());
inputs.Insert(maxTokensInput);

ScriptDialogInputText requestFileInput = new ScriptDialogInputText("Request file path", settings.GetRequestFilePath());
inputs.Insert(requestFileInput);

ScriptDialogInputText responseFileInput = new ScriptDialogInputText("Response file path", settings.GetResponseFilePath());
inputs.Insert(responseFileInput);

ScriptDialogInputCheckBox autoInsertInput = new ScriptDialogInputCheckBox("Insert generated code automatically", settings.GetAutoInsertCode());
inputs.Insert(autoInsertInput);

ScriptDialogInputCheckBox confirmInput = new ScriptDialogInputCheckBox("Show confirmation dialogs", settings.GetShowConfirmationDialogs());
inputs.Insert(confirmInput);

ScriptDialogInputCheckBox saveHistoryInput = new ScriptDialogInputCheckBox("Persist request history", settings.GetSaveRequestHistory());
inputs.Insert(saveHistoryInput);

ScriptDialogInputText historyInput = new ScriptDialogInputText("Maximum history entries", settings.GetMaxHistoryEntries().ToString());
inputs.Insert(historyInput);

bool confirmed = Workbench.ScriptDialog().Show("AI Copilot Settings", "Save", "Cancel", inputs);
m_IsSettingsDialogOpen = false;

if (!confirmed)
return;

settings.SetServiceProvider(serviceInput.GetValue());
settings.SetAPIKey(apiKeyInput.GetValue().Trim());
settings.SetModelName(modelInput.GetValue().Trim());
settings.SetCustomEndpoint(endpointInput.GetValue().Trim());
settings.SetTemperature(temperatureInput.GetValue().ToFloat());
settings.SetMaxTokens(maxTokensInput.GetValue().ToInt());
settings.SetAutoInsertCode(autoInsertInput.GetValue());
settings.SetShowConfirmationDialogs(confirmInput.GetValue());
settings.SetSaveRequestHistory(saveHistoryInput.GetValue());
settings.SetMaxHistoryEntries(historyInput.GetValue().ToInt());
settings.SetRequestFilePath(requestFileInput.GetValue().Trim());
settings.SetResponseFilePath(responseFileInput.GetValue().Trim());
        }

        //-----------------------------------------------------------------------------
        //! Process AI request from UI
        void ProcessAIRequest(AIRequestType requestType, string userInput)
        {
                if (m_AICore.IsProcessing())
                {
                        ShowMessage("AI Assistant is currently processing another request. Please wait...");
                        return;
                }

                WorkbenchContext context = GetCurrentWorkbenchContext();

                AIUIResponseCallback callback = new AIUIResponseCallback(this);

                m_AICore.ProcessRequest(requestType, userInput, context, callback);

                UpdateUIForProcessing();
        }

        //-----------------------------------------------------------------------------
        //! Acquire a fresh workbench context from the plugin
        protected WorkbenchContext GetCurrentWorkbenchContext()
        {
                if (m_Plugin)
                        return m_Plugin.GetCurrentContext();

                return new WorkbenchContext();
        }

        //-----------------------------------------------------------------------------
        //! Notify user while processing a request
protected void UpdateUIForProcessing()
{
Print("[AI Copilot] Processing request...");
}

        //-----------------------------------------------------------------------------
        //! Handle AI response and update UI
        void OnAIResponseReceived(string response)
        {
                ShowAIResponse(response);
                UpdateUIForReady();
        }

        //-----------------------------------------------------------------------------
        //! Handle AI error and update UI
        void OnAIErrorReceived(string error)
        {
                ShowMessage("AI error: " + error);
                UpdateUIForReady();
        }

        //-----------------------------------------------------------------------------
        //! Show AI response in UI
protected void ShowAIResponse(string response)
{
Workbench.Dialog("AI Copilot Response", response, MessageBoxButtons.OK);
}

        //-----------------------------------------------------------------------------
        //! Update UI when processing completes
protected void UpdateUIForReady()
{
Print("[AI Copilot] Ready");
}

        //-----------------------------------------------------------------------------
        //! Show message to user
        protected void ShowMessage(string message)
        {
                Workbench.Dialog("AI Copilot", message, MessageBoxButtons.OK);
        }

        //-----------------------------------------------------------------------------
        //! Show request history
        void ShowRequestHistory()
        {
                array<ref AIRequest> history = m_AICore.GetRequestHistory();

                string historyText = "=== Request History ===\n\n";

                if (history.Count() == 0)
                {
                        historyText += "No previous requests.";
                }
                else
                {
                        for (int i = history.Count() - 1; i >= 0; i--)
                        {
                                AIRequest request = history[i];
                                historyText += "Request: " + request.userInput + "\n";
                                historyText += "Type: " + EnumToString(typeof(AIRequestType), request.type) + "\n";
                                historyText += "Completed: " + (request.isCompleted ? "Yes" : "No") + "\n";

                                if (!request.response.IsEmpty())
                                        historyText += "Response: " + request.response + "\n";

                                if (!request.errorMessage.IsEmpty())
                                        historyText += "Error: " + request.errorMessage + "\n";

                                historyText += "---\n";
                        }
                }

Workbench.Dialog("AI Copilot History", historyText, MessageBoxButtons.OK);
}
}

//-----------------------------------------------------------------------------
//! Response callback for UI updates
class AIUIResponseCallback : AIResponseCallback
{
        protected AIAssistantUI m_UI;

        void AIUIResponseCallback(AIAssistantUI ui)
        {
                m_UI = ui;
        }

        override void OnSuccess(string response)
        {
                m_UI.OnAIResponseReceived(response);
        }

        override void OnError(string error)
        {
                m_UI.OnAIErrorReceived(error);
        }
}

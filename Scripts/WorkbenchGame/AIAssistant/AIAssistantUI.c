//-----------------------------------------------------------------------------
//! User Interface for AI Assistant plugin
//-----------------------------------------------------------------------------

class AIAssistantUI
{
	protected ref AIAssistantCore m_AICore;
	protected Widget m_MainDialog;
	protected Widget m_SettingsDialog;
	protected bool m_IsMainDialogOpen;
	protected bool m_IsSettingsDialogOpen;
	
	// Main dialog widgets
	protected EditBoxWidget m_RequestInput;
	protected MultilineEditBoxWidget m_ResponseOutput;
	protected ButtonWidget m_GenerateCodeBtn;
	protected ButtonWidget m_AnalyzeCodeBtn;
	protected ButtonWidget m_DebugCodeBtn;
	protected ButtonWidget m_DocumentCodeBtn;
	protected ButtonWidget m_OptimizeCodeBtn;
	protected ButtonWidget m_ExplainCodeBtn;
	protected ButtonWidget m_RefactorCodeBtn;
	protected ButtonWidget m_SettingsBtn;
	protected ButtonWidget m_ClearBtn;
	protected ButtonWidget m_HistoryBtn;
	protected TextWidget m_StatusText;
	
	// Settings dialog widgets
	protected EditBoxWidget m_APIKeyInput;
	protected ComboBoxWidget m_ServiceProviderCombo;
	protected EditBoxWidget m_ModelNameInput;
	protected EditBoxWidget m_CustomEndpointInput;
	protected CheckBoxWidget m_AutoInsertCheck;
	protected CheckBoxWidget m_ShowDialogsCheck;
	protected CheckBoxWidget m_SaveHistoryCheck;
	protected SpinBoxWidget m_MaxHistorySpinner;
	
	//-----------------------------------------------------------------------------
	void AIAssistantUI(AIAssistantCore aiCore)
	{
		m_AICore = aiCore;
		m_IsMainDialogOpen = false;
		m_IsSettingsDialogOpen = false;
	}
	
	//-----------------------------------------------------------------------------
	//! Show main AI Assistant dialog
	void ShowMainDialog()
	{
		if (m_IsMainDialogOpen)
			return;
		
		// Create main dialog using Workbench dialog system
		CreateMainDialog();
		m_IsMainDialogOpen = true;
	}
	
	//-----------------------------------------------------------------------------
	//! Show settings dialog
	void ShowSettingsDialog()
	{
		if (m_IsSettingsDialogOpen)
			return;
		
		CreateSettingsDialog();
		m_IsSettingsDialogOpen = true;
	}
	
	//-----------------------------------------------------------------------------
	//! Create main dialog interface
	protected void CreateMainDialog()
	{
		// Use Workbench dialog creation
		// This would typically load from a .layout file, but creating programmatically for demo
		
		string dialogContent = CreateMainDialogContent();
		
		// Show the dialog (simplified - actual implementation would use proper dialog system)
		if (Workbench.Dialog("AI Assistant", dialogContent, MessageBoxButtons.OK) == IDOK)
		{
			m_IsMainDialogOpen = false;
		}
	}
	
	//-----------------------------------------------------------------------------
	//! Create settings dialog interface
	protected void CreateSettingsDialog()
	{
		string settingsContent = CreateSettingsDialogContent();
		
		// Show settings dialog
		if (Workbench.Dialog("AI Assistant Settings", settingsContent, MessageBoxButtons.OKCANCEL) == IDOK)
		{
			// Save settings if OK was clicked
			SaveSettingsFromDialog();
		}
		
		m_IsSettingsDialogOpen = false;
	}
	
	//-----------------------------------------------------------------------------
	//! Generate main dialog content
	protected string CreateMainDialogContent()
	{
		string content = "=== AI Assistant for Arma Reforger ===\n\n";
		
		content += "Select an AI function:\n\n";
		content += "[Generate Code] - Create new code from description\n";
		content += "[Analyze Code] - Review selected code for issues\n";
		content += "[Debug Code] - Help debug problematic code\n";
		content += "[Document Code] - Generate documentation\n";
		content += "[Optimize Code] - Suggest performance improvements\n";
		content += "[Explain Code] - Explain how code works\n";
		content += "[Refactor Code] - Improve code structure\n\n";
		
		content += "Current Status: ";
		if (m_AICore.IsProcessing())
		{
			content += "Processing request...";
		}
		else
		{
			content += "Ready";
		}
		
		content += "\n\nEnter your request or question:\n";
		content += "[Text input would go here]\n\n";
		
		if (m_AICore.GetRequestHistory().Count() > 0)
		{
			content += "Recent Requests:\n";
			array<ref AIRequest> history = m_AICore.GetRequestHistory();
			int maxShow = Math.Min(3, history.Count());
			
			for (int i = history.Count() - maxShow; i < history.Count(); i++)
			{
				AIRequest request = history[i];
				content += "- " + request.userInput.Substring(0, Math.Min(50, request.userInput.Length()));
				if (request.userInput.Length() > 50)
					content += "...";
				content += "\n";
			}
		}
		
		return content;
	}
	
	//-----------------------------------------------------------------------------
	//! Generate settings dialog content
	protected string CreateSettingsDialogContent()
	{
		AIAssistantSettings settings = m_AICore.m_Settings;
		
		string content = "=== AI Assistant Settings ===\n\n";
		
		content += "API Configuration:\n";
		content += "Service Provider: ";
		switch (settings.GetServiceProvider())
		{
			case AIServiceProvider.CLAUDE_API:
				content += "Claude API";
				break;
			case AIServiceProvider.OPENAI_API:
				content += "OpenAI API";
				break;
			case AIServiceProvider.LOCAL_MODEL:
				content += "Local Model";
				break;
			case AIServiceProvider.CUSTOM_ENDPOINT:
				content += "Custom Endpoint";
				break;
		}
		content += "\n";
		
		content += "API Key: " + (settings.GetAPIKey().IsEmpty() ? "[Not Set]" : "[Configured]") + "\n";
		content += "Model Name: " + settings.GetModelName() + "\n";
		
		if (settings.GetServiceProvider() == AIServiceProvider.CUSTOM_ENDPOINT)
		{
			content += "Custom Endpoint: " + settings.GetCustomEndpoint() + "\n";
		}
		
		content += "\nBehavior Settings:\n";
		content += "Auto-insert generated code: " + (settings.GetAutoInsertCode() ? "Yes" : "No") + "\n";
		content += "Show confirmation dialogs: " + (settings.GetShowConfirmationDialogs() ? "Yes" : "No") + "\n";
		content += "Save request history: " + (settings.GetSaveRequestHistory() ? "Yes" : "No") + "\n";
		content += "Max history entries: " + settings.GetMaxHistoryEntries() + "\n";
		content += "Code style: " + settings.GetCodeStyle() + "\n";
		
		content += "\nUI Settings:\n";
		content += "Show tooltips: " + (settings.GetShowTooltips() ? "Yes" : "No") + "\n";
		content += "Theme preference: " + settings.GetThemePreference() + "\n";
		
		content += "\n[In a real implementation, these would be interactive controls]";
		
		return content;
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
		
		if (userInput.IsEmpty())
		{
			ShowMessage("Please enter a request or question.");
			return;
		}
		
		// Get current workbench context
		WorkbenchContext context = GetCurrentWorkbenchContext();
		
		// Create response callback
		AIUIResponseCallback callback = new AIUIResponseCallback(this);
		
		// Process the request
		m_AICore.ProcessRequest(requestType, userInput, context, callback);
		
		// Update UI to show processing state
		UpdateUIForProcessing();
	}
	
	//-----------------------------------------------------------------------------
	//! Get current workbench context for requests
	protected WorkbenchContext GetCurrentWorkbenchContext()
	{
		WorkbenchContext context = new WorkbenchContext();
		
		// This would integrate with actual Workbench API to get context
		// For now, create a basic context
		context.currentModule = "ScriptEditor"; // Placeholder
		context.currentScript = ""; // Would get from actual editor
		
		return context;
	}
	
	//-----------------------------------------------------------------------------
	//! Update UI when processing starts
	protected void UpdateUIForProcessing()
	{
		// Update status text and disable buttons while processing
		// In real implementation, would update actual UI widgets
	}
	
	//-----------------------------------------------------------------------------
	//! Handle AI response and update UI
	void OnAIResponseReceived(string response)
	{
		// Update response output widget
		ShowAIResponse(response);
		
		// Re-enable UI controls
		UpdateUIForReady();
	}
	
	//-----------------------------------------------------------------------------
	//! Handle AI error and update UI
	void OnAIErrorReceived(string error)
	{
		ShowMessage("AI Error: " + error);
		UpdateUIForReady();
	}
	
	//-----------------------------------------------------------------------------
	//! Show AI response in UI
	protected void ShowAIResponse(string response)
	{
		// In real implementation, would populate the response text widget
		// For now, show in a dialog
		Workbench.Dialog("AI Response", response, MessageBoxButtons.OK);
	}
	
	//-----------------------------------------------------------------------------
	//! Update UI when processing completes
	protected void UpdateUIForReady()
	{
		// Re-enable buttons and update status
		// In real implementation, would update actual UI widgets
	}
	
	//-----------------------------------------------------------------------------
	//! Show message to user
	protected void ShowMessage(string message)
	{
		Workbench.Dialog("AI Assistant", message, MessageBoxButtons.OK);
	}
	
	//-----------------------------------------------------------------------------
	//! Save settings from dialog controls
	protected void SaveSettingsFromDialog()
	{
		// In real implementation, would read values from actual dialog controls
		// and save them using the settings object
	}
	
	//-----------------------------------------------------------------------------
	//! Insert code into current script editor
	void InsertCodeIntoEditor(string code)
	{
		// This would integrate with the actual Script Editor API
		// to insert generated code at the cursor position
		
		// For now, just copy to clipboard or show in dialog
		ShowAIResponse("Generated Code:\n\n" + code);
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
				
				if (!request.errorMessage.IsEmpty())
				{
					historyText += "Error: " + request.errorMessage + "\n";
				}
				
				historyText += "---\n";
			}
		}
		
		Workbench.Dialog("Request History", historyText, MessageBoxButtons.OK);
	}
	
	//-----------------------------------------------------------------------------
	//! Clear response output
	void ClearResponse()
	{
		// Clear the response output widget
		// In real implementation, would clear actual UI widget
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
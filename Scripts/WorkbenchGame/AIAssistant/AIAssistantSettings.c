//-----------------------------------------------------------------------------
//! Settings management for AI Assistant plugin
//-----------------------------------------------------------------------------

enum AIServiceProvider
{
	CLAUDE_API,
	OPENAI_API,
	LOCAL_MODEL,
	CUSTOM_ENDPOINT
}

class AIAssistantSettings
{
	protected string m_ConfigPath;
	protected bool m_IsConfigured;
	
	// API Settings
	protected AIServiceProvider m_ServiceProvider;
	protected string m_APIKey;
	protected string m_CustomEndpoint;
	protected string m_ModelName;
	
	// Behavior Settings
	protected bool m_AutoInsertCode;
	protected bool m_ShowConfirmationDialogs;
	protected bool m_SaveRequestHistory;
	protected int m_MaxHistoryEntries;
	protected string m_CodeStyle;
	
	// UI Settings
	protected bool m_ShowTooltips;
	protected string m_ThemePreference;
	
	//-----------------------------------------------------------------------------
	void AIAssistantSettings()
	{
		m_ConfigPath = "$profile:AIAssistantConfig.json";
		m_IsConfigured = false;
		
		// Default settings
		m_ServiceProvider = AIServiceProvider.CLAUDE_API;
		m_APIKey = "";
		m_CustomEndpoint = "";
		m_ModelName = "claude-3-sonnet-20240229";
		
		m_AutoInsertCode = false;
		m_ShowConfirmationDialogs = true;
		m_SaveRequestHistory = true;
		m_MaxHistoryEntries = 100;
		m_CodeStyle = "Standard";
		
		m_ShowTooltips = true;
		m_ThemePreference = "Dark";
		
		LoadSettings();
	}
	
	//-----------------------------------------------------------------------------
	//! Load settings from file
	void LoadSettings()
	{
		FileHandle file = FileIO.OpenFile(m_ConfigPath, FileMode.READ);
		if (!file)
		{
			// First time setup - save defaults
			SaveSettings();
			return;
		}
		
		string jsonContent = "";
		string line = "";
		while (file.ReadLine(line) != 0)
		{
			jsonContent += line + "\n";
		}
		file.Close();
		
		if (!jsonContent.IsEmpty())
		{
			ParseSettingsFromJSON(jsonContent);
			m_IsConfigured = !m_APIKey.IsEmpty();
		}
	}
	
	//-----------------------------------------------------------------------------
	//! Save settings to file
	void SaveSettings()
	{
		string jsonContent = GenerateSettingsJSON();
		
		FileHandle file = FileIO.OpenFile(m_ConfigPath, FileMode.WRITE);
		if (file)
		{
			file.Write(jsonContent);
			file.Close();
		}
	}
	
	//-----------------------------------------------------------------------------
	//! Generate JSON from current settings
	protected string GenerateSettingsJSON()
	{
		string json = "{\n";
		json += "  \"api_settings\": {\n";
		json += "    \"service_provider\": " + EnumToString(typeof(AIServiceProvider), m_ServiceProvider) + ",\n";
		json += "    \"api_key\": \"" + m_APIKey + "\",\n";
		json += "    \"custom_endpoint\": \"" + m_CustomEndpoint + "\",\n";
		json += "    \"model_name\": \"" + m_ModelName + "\"\n";
		json += "  },\n";
		json += "  \"behavior_settings\": {\n";
		json += "    \"auto_insert_code\": " + (m_AutoInsertCode ? "true" : "false") + ",\n";
		json += "    \"show_confirmation_dialogs\": " + (m_ShowConfirmationDialogs ? "true" : "false") + ",\n";
		json += "    \"save_request_history\": " + (m_SaveRequestHistory ? "true" : "false") + ",\n";
		json += "    \"max_history_entries\": " + m_MaxHistoryEntries + ",\n";
		json += "    \"code_style\": \"" + m_CodeStyle + "\"\n";
		json += "  },\n";
		json += "  \"ui_settings\": {\n";
		json += "    \"show_tooltips\": " + (m_ShowTooltips ? "true" : "false") + ",\n";
		json += "    \"theme_preference\": \"" + m_ThemePreference + "\"\n";
		json += "  }\n";
		json += "}";
		
		return json;
	}
	
	//-----------------------------------------------------------------------------
	//! Parse settings from JSON (simplified parser)
	protected void ParseSettingsFromJSON(string jsonContent)
	{
		// Simplified JSON parsing - in production, would use proper JSON parser
		// This is a basic implementation for demonstration
		
		if (jsonContent.Contains("\"api_key\""))
		{
			int keyStart = jsonContent.IndexOf("\"api_key\": \"") + 12;
			int keyEnd = jsonContent.IndexOf("\"", keyStart);
			if (keyEnd > keyStart)
			{
				m_APIKey = jsonContent.Substring(keyStart, keyEnd - keyStart);
			}
		}
		
		if (jsonContent.Contains("\"model_name\""))
		{
			int modelStart = jsonContent.IndexOf("\"model_name\": \"") + 15;
			int modelEnd = jsonContent.IndexOf("\"", modelStart);
			if (modelEnd > modelStart)
			{
				m_ModelName = jsonContent.Substring(modelStart, modelEnd - modelStart);
			}
		}
		
		if (jsonContent.Contains("\"custom_endpoint\""))
		{
			int endpointStart = jsonContent.IndexOf("\"custom_endpoint\": \"") + 20;
			int endpointEnd = jsonContent.IndexOf("\"", endpointStart);
			if (endpointEnd > endpointStart)
			{
				m_CustomEndpoint = jsonContent.Substring(endpointStart, endpointEnd - endpointStart);
			}
		}
		
		// Parse boolean values
		m_AutoInsertCode = jsonContent.Contains("\"auto_insert_code\": true");
		m_ShowConfirmationDialogs = !jsonContent.Contains("\"show_confirmation_dialogs\": false");
		m_SaveRequestHistory = !jsonContent.Contains("\"save_request_history\": false");
		m_ShowTooltips = !jsonContent.Contains("\"show_tooltips\": false");
		
		// Parse numeric values
		if (jsonContent.Contains("\"max_history_entries\":"))
		{
			int numStart = jsonContent.IndexOf("\"max_history_entries\": ") + 24;
			int numEnd = jsonContent.IndexOf(",", numStart);
			if (numEnd == -1) numEnd = jsonContent.IndexOf("}", numStart);
			
			if (numEnd > numStart)
			{
				string numStr = jsonContent.Substring(numStart, numEnd - numStart).Trim();
				m_MaxHistoryEntries = numStr.ToInt();
			}
		}
	}
	
	//-----------------------------------------------------------------------------
	//! Getters and Setters
	bool IsConfigured() { return m_IsConfigured; }
	
	AIServiceProvider GetServiceProvider() { return m_ServiceProvider; }
	void SetServiceProvider(AIServiceProvider provider) 
	{ 
		m_ServiceProvider = provider; 
		SaveSettings();
	}
	
	string GetAPIKey() { return m_APIKey; }
	void SetAPIKey(string apiKey) 
	{ 
		m_APIKey = apiKey; 
		m_IsConfigured = !apiKey.IsEmpty();
		SaveSettings();
	}
	
	string GetCustomEndpoint() { return m_CustomEndpoint; }
	void SetCustomEndpoint(string endpoint) 
	{ 
		m_CustomEndpoint = endpoint; 
		SaveSettings();
	}
	
	string GetModelName() { return m_ModelName; }
	void SetModelName(string modelName) 
	{ 
		m_ModelName = modelName; 
		SaveSettings();
	}
	
	bool GetAutoInsertCode() { return m_AutoInsertCode; }
	void SetAutoInsertCode(bool autoInsert) 
	{ 
		m_AutoInsertCode = autoInsert; 
		SaveSettings();
	}
	
	bool GetShowConfirmationDialogs() { return m_ShowConfirmationDialogs; }
	void SetShowConfirmationDialogs(bool showDialogs) 
	{ 
		m_ShowConfirmationDialogs = showDialogs; 
		SaveSettings();
	}
	
	bool GetSaveRequestHistory() { return m_SaveRequestHistory; }
	void SetSaveRequestHistory(bool saveHistory) 
	{ 
		m_SaveRequestHistory = saveHistory; 
		SaveSettings();
	}
	
	int GetMaxHistoryEntries() { return m_MaxHistoryEntries; }
	void SetMaxHistoryEntries(int maxEntries) 
	{ 
		m_MaxHistoryEntries = maxEntries; 
		SaveSettings();
	}
	
	string GetCodeStyle() { return m_CodeStyle; }
	void SetCodeStyle(string codeStyle) 
	{ 
		m_CodeStyle = codeStyle; 
		SaveSettings();
	}
	
	bool GetShowTooltips() { return m_ShowTooltips; }
	void SetShowTooltips(bool showTooltips) 
	{ 
		m_ShowTooltips = showTooltips; 
		SaveSettings();
	}
	
	string GetThemePreference() { return m_ThemePreference; }
	void SetThemePreference(string theme) 
	{ 
		m_ThemePreference = theme; 
		SaveSettings();
	}
	
	//-----------------------------------------------------------------------------
	//! Reset to defaults
	void ResetToDefaults()
	{
		m_ServiceProvider = AIServiceProvider.CLAUDE_API;
		m_APIKey = "";
		m_CustomEndpoint = "";
		m_ModelName = "claude-3-sonnet-20240229";
		
		m_AutoInsertCode = false;
		m_ShowConfirmationDialogs = true;
		m_SaveRequestHistory = true;
		m_MaxHistoryEntries = 100;
		m_CodeStyle = "Standard";
		
		m_ShowTooltips = true;
		m_ThemePreference = "Dark";
		
		m_IsConfigured = false;
		SaveSettings();
	}
	
	//-----------------------------------------------------------------------------
	//! Validate current settings
	bool ValidateSettings()
	{
		if (m_APIKey.IsEmpty() && m_ServiceProvider != AIServiceProvider.LOCAL_MODEL)
		{
			return false;
		}
		
		if (m_ServiceProvider == AIServiceProvider.CUSTOM_ENDPOINT && m_CustomEndpoint.IsEmpty())
		{
			return false;
		}
		
		if (m_MaxHistoryEntries < 0 || m_MaxHistoryEntries > 1000)
		{
			return false;
		}
		
		return true;
	}
}
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
protected float m_Temperature;
protected int m_MaxTokens;
protected string m_RequestFilePath;
protected string m_ResponseFilePath;
	
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
m_Temperature = 0.3;
m_MaxTokens = 4000;
m_RequestFilePath = "$profile:ai_request.json";
m_ResponseFilePath = "$profile:ai_response.json";
		
		m_AutoInsertCode = false;
		m_ShowConfirmationDialogs = true;
		m_SaveRequestHistory = true;
		m_MaxHistoryEntries = 100;
		m_CodeStyle = "Standard";
		
		m_ShowTooltips = true;
		m_ThemePreference = "Dark";
		
LoadSettings();
UpdateConfiguredState();
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
UpdateConfiguredState();
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
string providerLabel = EnumToString(typeof(AIServiceProvider), m_ServiceProvider);
json += "    \"service_provider\": \"" + providerLabel + "\",\n";
json += "    \"api_key\": \"" + m_APIKey + "\",\n";
json += "    \"custom_endpoint\": \"" + m_CustomEndpoint + "\",\n";
json += "    \"model_name\": \"" + m_ModelName + "\",\n";
json += "    \"temperature\": " + m_Temperature + ",\n";
json += "    \"max_tokens\": " + m_MaxTokens + ",\n";
json += "    \"request_file\": \"" + m_RequestFilePath + "\",\n";
json += "    \"response_file\": \"" + m_ResponseFilePath + "\"\n";
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

if (jsonContent.Contains("CLAUDE_API"))
{
m_ServiceProvider = AIServiceProvider.CLAUDE_API;
}
else if (jsonContent.Contains("OPENAI_API"))
{
m_ServiceProvider = AIServiceProvider.OPENAI_API;
}
else if (jsonContent.Contains("LOCAL_MODEL"))
{
m_ServiceProvider = AIServiceProvider.LOCAL_MODEL;
}
else if (jsonContent.Contains("CUSTOM_ENDPOINT"))
{
m_ServiceProvider = AIServiceProvider.CUSTOM_ENDPOINT;
}
		
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

if (jsonContent.Contains("\"temperature\":"))
{
int tempStart = jsonContent.IndexOf("\"temperature\": ") + 15;
int tempEnd = jsonContent.IndexOf(",", tempStart);
if (tempEnd == -1) tempEnd = jsonContent.IndexOf("\n", tempStart);
if (tempEnd == -1) tempEnd = jsonContent.Length();

if (tempEnd > tempStart)
{
string tempStr = jsonContent.Substring(tempStart, tempEnd - tempStart).Trim();
m_Temperature = tempStr.ToFloat();
}
}

if (jsonContent.Contains("\"max_tokens\":"))
{
int tokensStart = jsonContent.IndexOf("\"max_tokens\": ") + 15;
int tokensEnd = jsonContent.IndexOf(",", tokensStart);
if (tokensEnd == -1) tokensEnd = jsonContent.IndexOf("\n", tokensStart);
if (tokensEnd == -1) tokensEnd = jsonContent.Length();

if (tokensEnd > tokensStart)
{
string tokensStr = jsonContent.Substring(tokensStart, tokensEnd - tokensStart).Trim();
m_MaxTokens = tokensStr.ToInt();
}
}

if (jsonContent.Contains("\"request_file\""))
{
int reqStart = jsonContent.IndexOf("\"request_file\": \"") + 18;
int reqEnd = jsonContent.IndexOf("\"", reqStart);
if (reqEnd > reqStart)
m_RequestFilePath = jsonContent.Substring(reqStart, reqEnd - reqStart);
}

if (jsonContent.Contains("\"response_file\""))
{
int resStart = jsonContent.IndexOf("\"response_file\": \"") + 19;
int resEnd = jsonContent.IndexOf("\"", resStart);
if (resEnd > resStart)
m_ResponseFilePath = jsonContent.Substring(resStart, resEnd - resStart);
}
}
	
	//-----------------------------------------------------------------------------
	//! Getters and Setters
	bool IsConfigured() { return m_IsConfigured; }
	
AIServiceProvider GetServiceProvider() { return m_ServiceProvider; }
void SetServiceProvider(AIServiceProvider provider)
{
m_ServiceProvider = provider;
UpdateConfiguredState();
SaveSettings();
}
	
string GetAPIKey() { return m_APIKey; }
void SetAPIKey(string apiKey)
{
m_APIKey = apiKey;
UpdateConfiguredState();
SaveSettings();
}
	
string GetCustomEndpoint() { return m_CustomEndpoint; }
void SetCustomEndpoint(string endpoint)
{
m_CustomEndpoint = endpoint;
UpdateConfiguredState();
SaveSettings();
}
	
string GetModelName() { return m_ModelName; }
void SetModelName(string modelName)
{
m_ModelName = modelName;
SaveSettings();
}

float GetTemperature() { return m_Temperature; }
void SetTemperature(float temperature)
{
if (temperature < 0.0)
temperature = 0.0;
if (temperature > 2.0)
temperature = 2.0;

m_Temperature = temperature;
SaveSettings();
}

int GetMaxTokens() { return m_MaxTokens; }
void SetMaxTokens(int maxTokens)
{
if (maxTokens < 64)
maxTokens = 64;
if (maxTokens > 60000)
maxTokens = 60000;

m_MaxTokens = maxTokens;
SaveSettings();
}

string GetRequestFilePath() { return m_RequestFilePath; }
void SetRequestFilePath(string path)
{
if (!path.IsEmpty())
{
m_RequestFilePath = path;
SaveSettings();
}
}

string GetResponseFilePath() { return m_ResponseFilePath; }
void SetResponseFilePath(string path)
{
if (!path.IsEmpty())
{
m_ResponseFilePath = path;
SaveSettings();
}
}

string GetServiceIdentifier()
{
switch (m_ServiceProvider)
{
case AIServiceProvider.CLAUDE_API:
return "claude";

case AIServiceProvider.OPENAI_API:
return "openai";

case AIServiceProvider.LOCAL_MODEL:
return "ollama";

case AIServiceProvider.CUSTOM_ENDPOINT:
return "custom";
}

return "claude";
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

protected void UpdateConfiguredState()
{
switch (m_ServiceProvider)
{
case AIServiceProvider.LOCAL_MODEL:
m_IsConfigured = true;
break;

case AIServiceProvider.CUSTOM_ENDPOINT:
m_IsConfigured = !m_CustomEndpoint.IsEmpty();
break;

default:
m_IsConfigured = !m_APIKey.IsEmpty();
break;
}
}

//-----------------------------------------------------------------------------
//! Reset to defaults
void ResetToDefaults()
{
m_ServiceProvider = AIServiceProvider.CLAUDE_API;
m_APIKey = "";
m_CustomEndpoint = "";
m_ModelName = "claude-3-sonnet-20240229";
m_Temperature = 0.3;
m_MaxTokens = 4000;
m_RequestFilePath = "$profile:ai_request.json";
m_ResponseFilePath = "$profile:ai_response.json";

m_AutoInsertCode = false;
m_ShowConfirmationDialogs = true;
m_SaveRequestHistory = true;
m_MaxHistoryEntries = 100;
m_CodeStyle = "Standard";

m_ShowTooltips = true;
m_ThemePreference = "Dark";

UpdateConfiguredState();
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

if (m_RequestFilePath.IsEmpty() || m_ResponseFilePath.IsEmpty())
{
return false;
}

if (m_MaxTokens <= 0)
{
return false;
}

if (m_Temperature < 0.0 || m_Temperature > 2.0)
{
return false;
}

return true;
}
}
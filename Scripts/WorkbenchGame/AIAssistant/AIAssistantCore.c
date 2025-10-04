//-----------------------------------------------------------------------------
//! Core AI Assistant functionality
//! Handles communication with AI services and processing requests
//-----------------------------------------------------------------------------

enum AIRequestType
{
GENERAL_CHAT,
CODE_GENERATION,
CODE_ANALYSIS,
CODE_DEBUGGING,
DOCUMENTATION,
OPTIMIZATION,
	EXPLANATION,
	REFACTORING
}

class AIAssistantCore
{
	protected ref AIAssistantSettings m_Settings;
	protected ref array<ref AIRequest> m_RequestHistory;
protected bool m_IsProcessing;
protected AIServiceCallback m_PendingServiceCallback;
protected AIRequest m_ActiveRequest;
protected int m_RequestStartTick;
protected int m_ResponseTimeoutMs;
protected int m_PollIntervalMs;
	
	//-----------------------------------------------------------------------------
void AIAssistantCore(AIAssistantSettings settings)
{
m_Settings = settings;
m_RequestHistory = {};
m_IsProcessing = false;
m_PendingServiceCallback = null;
m_ActiveRequest = null;
m_RequestStartTick = 0;
m_ResponseTimeoutMs = 60000;
m_PollIntervalMs = 500;
}
	
	//-----------------------------------------------------------------------------
	//! Process AI request with context
	void ProcessRequest(AIRequestType requestType, string userInput, WorkbenchContext context, AIResponseCallback callback)
	{
		if (m_IsProcessing)
		{
			callback.OnError("AI Assistant is currently processing another request. Please wait...");
			return;
		}
		
		m_IsProcessing = true;
		
		// Create request object
		AIRequest request = new AIRequest();
		request.type = requestType;
		request.userInput = userInput;
		request.context = context;
		request.timestamp = System.GetTickCount();
		
// Add to history
m_RequestHistory.Insert(request);
ManageHistory(request);
m_ActiveRequest = request;
		
		// Process based on request type
switch (requestType)
{
case AIRequestType.GENERAL_CHAT:
ProcessGeneralChat(request, callback);
break;

case AIRequestType.CODE_GENERATION:
ProcessCodeGeneration(request, callback);
break;
				
			case AIRequestType.CODE_ANALYSIS:
				ProcessCodeAnalysis(request, callback);
				break;
				
			case AIRequestType.CODE_DEBUGGING:
				ProcessDebugging(request, callback);
				break;
				
			case AIRequestType.DOCUMENTATION:
				ProcessDocumentation(request, callback);
				break;
				
			case AIRequestType.OPTIMIZATION:
				ProcessOptimization(request, callback);
				break;
				
			case AIRequestType.EXPLANATION:
				ProcessExplanation(request, callback);
				break;
				
case AIRequestType.REFACTORING:
ProcessRefactoring(request, callback);
break;

default:
callback.OnError("Unsupported request type");
m_IsProcessing = false;
}
	}
	
	//-----------------------------------------------------------------------------
	//! Generate code from natural language description
	protected void ProcessCodeGeneration(AIRequest request, AIResponseCallback callback)
	{
		string prompt = BuildCodeGenerationPrompt(request);
		
		// Send to AI service
		SendToAIService(prompt, new AICodeGenerationCallback(callback));
	}
	
	//-----------------------------------------------------------------------------
	//! Analyze existing code for issues
protected void ProcessCodeAnalysis(AIRequest request, AIResponseCallback callback)
	{
		string codeToAnalyze = GetSelectedCode(request.context);
		if (codeToAnalyze.IsEmpty())
		{
			callback.OnError("No code selected for analysis");
			m_IsProcessing = false;
			return;
		}
		
		string prompt = BuildCodeAnalysisPrompt(codeToAnalyze, request.userInput);
		SendToAIService(prompt, new AICodeAnalysisCallback(callback));
	}
	
	//-----------------------------------------------------------------------------
	//! Help debug code issues
	protected void ProcessDebugging(AIRequest request, AIResponseCallback callback)
	{
		string codeContext = GetSelectedCode(request.context);
		string errorInfo = request.userInput; // User describes the error
		
		string prompt = BuildDebuggingPrompt(codeContext, errorInfo);
		SendToAIService(prompt, new AIDebuggingCallback(callback));
	}
	
	//-----------------------------------------------------------------------------
	//! Generate documentation
	protected void ProcessDocumentation(AIRequest request, AIResponseCallback callback)
	{
		string codeToDocument = GetSelectedCode(request.context);
		if (codeToDocument.IsEmpty())
		{
			callback.OnError("No code selected for documentation");
			m_IsProcessing = false;
			return;
		}
		
		string prompt = BuildDocumentationPrompt(codeToDocument, request.userInput);
		SendToAIService(prompt, new AIDocumentationCallback(callback));
	}
	
	//-----------------------------------------------------------------------------
	//! Suggest optimizations
	protected void ProcessOptimization(AIRequest request, AIResponseCallback callback)
	{
		string codeToOptimize = GetSelectedCode(request.context);
		if (codeToOptimize.IsEmpty())
		{
			callback.OnError("No code selected for optimization");
			m_IsProcessing = false;
			return;
		}
		
		string prompt = BuildOptimizationPrompt(codeToOptimize, request.userInput);
		SendToAIService(prompt, new AIOptimizationCallback(callback));
	}
	
//----------------------------------------------------------------------------- 
//! Explain code functionality
protected void ProcessExplanation(AIRequest request, AIResponseCallback callback)
	{
		string codeToExplain = GetSelectedCode(request.context);
		if (codeToExplain.IsEmpty())
		{
			callback.OnError("No code selected for explanation");
			m_IsProcessing = false;
			return;
		}
		
		string prompt = BuildExplanationPrompt(codeToExplain, request.userInput);
		SendToAIService(prompt, new AIExplanationCallback(callback));
	}
	
//----------------------------------------------------------------------------- 
//! Refactor code
protected void ProcessRefactoring(AIRequest request, AIResponseCallback callback)
{
string codeToRefactor = GetSelectedCode(request.context);
if (codeToRefactor.IsEmpty())
{
callback.OnError("No code selected for refactoring");
m_IsProcessing = false;
return;
}

string prompt = BuildRefactoringPrompt(codeToRefactor, request.userInput);
SendToAIService(prompt, new AIRefactoringCallback(callback));
}

//----------------------------------------------------------------------------- 
//! General chat or contextual guidance
protected void ProcessGeneralChat(AIRequest request, AIResponseCallback callback)
{
string prompt = BuildGeneralChatPrompt(request);
SendToAIService(prompt, new AIChatCallback(callback));
}
	
	//-----------------------------------------------------------------------------
	//! Get selected code from context
string GetSelectedCode(WorkbenchContext context)
	{
		if (context.currentModule == "ScriptEditor" && !context.currentScript.IsEmpty())
		{
			// Get selected text or entire script
			return Workbench.ScriptDialog().GetSelectedText();
		}
		
		// Could extend to get code from selected resources, etc.
		return "";
	}
	
//----------------------------------------------------------------------------- 
//! Send request to AI service through the local bridge
protected void SendToAIService(string prompt, AIServiceCallback serviceCallback)
{
if (StartBridgeRequest(prompt, serviceCallback))
return;

string errorMessage = "Unable to communicate with AI bridge service.";
FinalizeRequestWithError(errorMessage, serviceCallback);
}

//----------------------------------------------------------------------------- 
//! Prepare request file and schedule polling for response
protected bool StartBridgeRequest(string prompt, AIServiceCallback serviceCallback)
{
if (!serviceCallback)
return false;

string requestJSON = BuildBridgeRequestJSON(prompt);
if (requestJSON.IsEmpty())
return false;

CleanupBridgeFiles();

if (!WriteFile(m_Settings.GetRequestFilePath(), requestJSON))
return false;

m_PendingServiceCallback = serviceCallback;
m_RequestStartTick = System.GetTickCount();
ScheduleBridgePoll();
return true;
}

//----------------------------------------------------------------------------- 
//! Schedule another poll for the AI bridge response file
protected void ScheduleBridgePoll()
{
GetGame().GetCallqueue().CallLater(CheckForBridgeResponse, m_PollIntervalMs, false);
}

//----------------------------------------------------------------------------- 
//! Check whether the AI bridge wrote the response file
protected void CheckForBridgeResponse()
{
if (!m_PendingServiceCallback)
return;

string responseContent;
if (!TryReadFile(m_Settings.GetResponseFilePath(), responseContent))
{
if (System.GetTickCount() - m_RequestStartTick >= m_ResponseTimeoutMs)
{
HandleBridgeError("Timed out waiting for AI bridge response.");
}
else
{
ScheduleBridgePoll();
}
return;
}

FileIO.DeleteFile(m_Settings.GetResponseFilePath());

string responseText;
string errorText;
if (ParseBridgeResponse(responseContent, responseText, errorText))
{
HandleBridgeSuccess(responseText);
}
else
{
HandleBridgeError(errorText);
}
}

//----------------------------------------------------------------------------- 
//! Handle successful response from bridge service
protected void HandleBridgeSuccess(string responseText)
{
CleanupBridgeFiles();

if (m_ActiveRequest)
{
m_ActiveRequest.response = responseText;
m_ActiveRequest.isCompleted = true;
m_ActiveRequest.errorMessage = "";
}

AIServiceCallback callback = m_PendingServiceCallback;
m_PendingServiceCallback = null;
m_IsProcessing = false;
m_ActiveRequest = null;

if (callback)
callback.OnSuccess(responseText);
}

//----------------------------------------------------------------------------- 
//! Handle bridge error
protected void HandleBridgeError(string errorMessage)
{
CleanupBridgeFiles();

if (m_ActiveRequest)
{
m_ActiveRequest.isCompleted = true;
m_ActiveRequest.errorMessage = errorMessage;
}

AIServiceCallback callback = m_PendingServiceCallback;
m_PendingServiceCallback = null;
m_IsProcessing = false;
m_ActiveRequest = null;

if (callback)
callback.OnError(errorMessage);
}

//----------------------------------------------------------------------------- 
//! Finalize immediately when bridge communication fails before scheduling
protected void FinalizeRequestWithError(string errorMessage, AIServiceCallback serviceCallback)
{
CleanupBridgeFiles();

if (m_ActiveRequest)
{
m_ActiveRequest.isCompleted = true;
m_ActiveRequest.errorMessage = errorMessage;
}

m_IsProcessing = false;
m_ActiveRequest = null;

if (serviceCallback)
serviceCallback.OnError(errorMessage);
}

//----------------------------------------------------------------------------- 
//! Remove any leftover request/response files to avoid stale data
protected void CleanupBridgeFiles()
{
DeleteFileIfExists(m_Settings.GetRequestFilePath());
DeleteFileIfExists(m_Settings.GetResponseFilePath());
}

protected void DeleteFileIfExists(string path)
{
FileHandle handle = FileIO.OpenFile(path, FileMode.READ);
if (!handle)
return;

handle.Close();
FileIO.DeleteFile(path);
}

//----------------------------------------------------------------------------- 
//! Read file content if available
protected bool TryReadFile(string path, out string content)
{
content = "";

FileHandle file = FileIO.OpenFile(path, FileMode.READ);
if (!file)
return false;

string line;
while (file.ReadLine(line) != 0)
{
content += line;
content += "\n";
}

file.Close();
return true;
}

//----------------------------------------------------------------------------- 
//! Write content to specified file path
protected bool WriteFile(string path, string content)
{
FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
if (!file)
return false;

file.Write(content);
file.Close();
return true;
}

//----------------------------------------------------------------------------- 
//! Parse JSON content from bridge response
protected bool ParseBridgeResponse(string jsonContent, out string responseText, out string errorText)
{
responseText = "";
errorText = "";

bool successFlag;
if (TryParseJSONBool(jsonContent, "success", successFlag) && !successFlag)
{
if (!TryParseJSONString(jsonContent, "error", errorText))
errorText = "AI bridge reported an unknown error.";
return false;
}

if (TryParseJSONString(jsonContent, "response", responseText) && !responseText.IsEmpty())
return true;

if (!TryParseJSONString(jsonContent, "error", errorText) || errorText.IsEmpty())
errorText = "AI bridge returned an empty response.";

return false;
}

//----------------------------------------------------------------------------- 
//! Extract string value from simple JSON
protected bool TryParseJSONString(string jsonContent, string key, out string value)
{
value = "";
string search = "\"" + key + "\"";
int keyIndex = jsonContent.IndexOf(search);
if (keyIndex == -1)
return false;

int colonIndex = jsonContent.IndexOf(":", keyIndex);
if (colonIndex == -1)
return false;

colonIndex++;
while (colonIndex < jsonContent.Length())
{
string ch = jsonContent.Substring(colonIndex, 1);
if (ch == " " || ch == "\t" || ch == "\n" || ch == "\r")
{
colonIndex++;
continue;
}

if (ch != "\"")
return false;

colonIndex++;
break;
}

string result = "";
bool escaping = false;

for (int i = colonIndex; i < jsonContent.Length(); i++)
{
string charStr = jsonContent.Substring(i, 1);
if (!escaping)
{
if (charStr == "\\")
{
escaping = true;
continue;
}

if (charStr == "\"")
{
value = result;
return true;
}

result += charStr;
}
else
{
if (charStr == "n")
result += "\n";
else if (charStr == "r")
result += "\r";
else if (charStr == "t")
result += "\t";
else if (charStr == "\"")
result += "\"";
else if (charStr == "\\")
result += "\\";
else
result += charStr;

escaping = false;
}
}

value = result;
return true;
}

//----------------------------------------------------------------------------- 
//! Extract boolean value from simple JSON
protected bool TryParseJSONBool(string jsonContent, string key, out bool value)
{
value = false;
string search = "\"" + key + "\"";
int keyIndex = jsonContent.IndexOf(search);
if (keyIndex == -1)
return false;

int colonIndex = jsonContent.IndexOf(":", keyIndex);
if (colonIndex == -1)
return false;

colonIndex++;
while (colonIndex < jsonContent.Length())
{
string token = jsonContent.Substring(colonIndex, 1);
if (token == " " || token == "\t" || token == "\n" || token == "\r")
{
colonIndex++;
continue;
}

string remainingTrue = jsonContent.Substring(colonIndex, Math.Min(4, jsonContent.Length() - colonIndex));
if (remainingTrue == "true")
{
value = true;
return true;
}

string remainingFalse = jsonContent.Substring(colonIndex, Math.Min(5, jsonContent.Length() - colonIndex));
if (remainingFalse == "false")
{
value = false;
return true;
}

break;
}

return false;
}
	
//----------------------------------------------------------------------------- 
//! Construct JSON payload for the bridge
protected string BuildBridgeRequestJSON(string prompt)
{
if (!m_ActiveRequest)
return "";

string service = m_Settings.GetServiceIdentifier();
if (service.IsEmpty())
return "";

string json = "{\n";
json += "  \"service\": \"" + service + "\",\n";
json += "  \"prompt\": \"" + EscapeJSONString(prompt) + "\",\n";
json += "  \"model\": \"" + EscapeJSONString(m_Settings.GetModelName()) + "\",\n";

ref array<string> settingsEntries = {};
settingsEntries.Insert("\"maxTokens\": " + m_Settings.GetMaxTokens());
settingsEntries.Insert("\"temperature\": " + m_Settings.GetTemperature());
settingsEntries.Insert("\"timeout\": " + m_ResponseTimeoutMs);
settingsEntries.Insert("\"request_file\": \"" + EscapeJSONString(m_Settings.GetRequestFilePath()) + "\"");
settingsEntries.Insert("\"response_file\": \"" + EscapeJSONString(m_Settings.GetResponseFilePath()) + "\"");

string apiKey = m_Settings.GetAPIKey();
if (!apiKey.IsEmpty())
{
settingsEntries.Insert("\"apiKey\": \"" + EscapeJSONString(apiKey) + "\"");
}

if (m_Settings.GetServiceProvider() == AIServiceProvider.CUSTOM_ENDPOINT && !m_Settings.GetCustomEndpoint().IsEmpty())
{
string endpointEscaped = EscapeJSONString(m_Settings.GetCustomEndpoint());
settingsEntries.Insert("\"endpoint\": \"" + endpointEscaped + "\"");
settingsEntries.Insert("\"customEndpoint\": \"" + endpointEscaped + "\"");
}

json += "  \"settings\": {\n";
for (int i = 0; i < settingsEntries.Count(); i++)
{
json += "    " + settingsEntries[i];
if (i < settingsEntries.Count() - 1)
json += ",\n";
else
json += "\n";
}
json += "  },\n";
json += "  \"metadata\": {\n";
json += "    \"requestType\": \"" + EscapeJSONString(EnumToString(typeof(AIRequestType), m_ActiveRequest.type)) + "\",\n";
json += "    \"context\": \"" + EscapeJSONString(BuildContextSummary(m_ActiveRequest)) + "\"\n";
json += "  }\n";
json += "}\n";

return json;
}

//----------------------------------------------------------------------------- 
//! Build short textual summary of workbench context
protected string BuildContextSummary(AIRequest request)
{
if (!request)
return "";

string summary = "Module=" + request.context.currentModule;

if (!request.context.currentScript.IsEmpty())
summary += ", Script=" + request.context.currentScript;

if (request.context.selectedResources && request.context.selectedResources.Count() > 0)
summary += ", Resources=" + request.context.selectedResources.Count().ToString();

if (request.context.selectedEntities && request.context.selectedEntities.Count() > 0)
summary += ", Entities=" + request.context.selectedEntities.Count().ToString();

return summary;
}

//----------------------------------------------------------------------------- 
//! Escape text for inclusion in JSON
protected string EscapeJSONString(string value)
{
string result = "";
for (int i = 0; i < value.Length(); i++)
{
string ch = value.Substring(i, 1);
switch (ch)
{
case "\\":
result += "\\\\";
break;
case "\"":
result += "\\\"";
break;
case "\n":
result += "\\n";
break;
case "\r":
result += "\\r";
break;
case "\t":
result += "\\t";
break;
default:
result += ch;
break;
}
}

return result;
}

//----------------------------------------------------------------------------- 
//! Maintain request history according to user settings
protected void ManageHistory(AIRequest request)
{
if (!m_Settings.GetSaveRequestHistory())
{
m_RequestHistory.Clear();
m_RequestHistory.Insert(request);
return;
}

int maxEntries = Math.Max(1, m_Settings.GetMaxHistoryEntries());
while (m_RequestHistory.Count() > maxEntries)
{
m_RequestHistory.RemoveOrdered(0);
}
}

//----------------------------------------------------------------------------- 
//! Build prompt for general conversation
protected string BuildGeneralChatPrompt(AIRequest request)
{
string prompt = "You are an AI copilot embedded in the Arma Reforger Workbench.\n";
prompt += "Assist with scripting, configuration and tooling questions.\n\n";
prompt += "User request:\n" + request.userInput + "\n\n";

string selectedCode = GetSelectedCode(request.context);
if (!selectedCode.IsEmpty())
{
prompt += "Selected code context:\n" + selectedCode + "\n\n";
}

prompt += "Workbench context: " + BuildContextSummary(request) + "\n";
return prompt;
}

//----------------------------------------------------------------------------- 
//! Build prompt for code generation
	protected string BuildCodeGenerationPrompt(AIRequest request)
	{
		string prompt = "Generate Arma Reforger Enforce Script code based on this request:\n\n";
		prompt += request.userInput + "\n\n";
		prompt += "Context:\n";
		prompt += "- Current module: " + request.context.currentModule + "\n";
		prompt += "- Use proper Enforce Script syntax\n";
		prompt += "- Follow Arma Reforger coding conventions\n";
		prompt += "- Include appropriate comments\n";
		prompt += "- Ensure code is production-ready\n";
		
		return prompt;
	}
	
	//-----------------------------------------------------------------------------
	//! Build prompt for code analysis
	protected string BuildCodeAnalysisPrompt(string code, string userRequest)
	{
		string prompt = "Analyze this Arma Reforger Enforce Script code:\n\n";
		prompt += code + "\n\n";
		prompt += "Focus on: " + userRequest + "\n\n";
		prompt += "Please provide:\n";
		prompt += "- Code quality assessment\n";
		prompt += "- Potential bugs or issues\n";
		prompt += "- Performance considerations\n";
		prompt += "- Best practice recommendations\n";
		
		return prompt;
	}
	
	//-----------------------------------------------------------------------------
	//! Build other prompt methods...
	protected string BuildDebuggingPrompt(string code, string errorInfo)
	{
		return "Debug this Arma Reforger code:\n\n" + code + "\n\nError/Issue: " + errorInfo;
	}
	
	protected string BuildDocumentationPrompt(string code, string docType)
	{
		return "Generate documentation for this Arma Reforger code:\n\n" + code + "\n\nDocumentation type: " + docType;
	}
	
	protected string BuildOptimizationPrompt(string code, string focusArea)
	{
		return "Optimize this Arma Reforger code:\n\n" + code + "\n\nOptimization focus: " + focusArea;
	}
	
	protected string BuildExplanationPrompt(string code, string question)
	{
		return "Explain this Arma Reforger code:\n\n" + code + "\n\nSpecific question: " + question;
	}
	
	protected string BuildRefactoringPrompt(string code, string refactorGoal)
	{
		return "Refactor this Arma Reforger code:\n\n" + code + "\n\nRefactoring goal: " + refactorGoal;
	}
	
//----------------------------------------------------------------------------- 
//! Expose active settings
AIAssistantSettings GetSettings()
{
return m_Settings;
}

//----------------------------------------------------------------------------- 
//! Get request history
array<ref AIRequest> GetRequestHistory()
{
return m_RequestHistory;
}
	
	//-----------------------------------------------------------------------------
	//! Check if currently processing
	bool IsProcessing()
	{
		return m_IsProcessing;
	}
}
import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.Separator
import com.intellij.openapi.ui.Messages
import liveplugin.*
import liveplugin.PluginUtil.currentEditorIn

registerAction(id = "Show Actions RWP", keyStroke = "ctrl shift H") { event: AnActionEvent ->
    val editor = event.project?.let { it1 -> currentEditorIn(it1) }
    val text = editor?.selectionModel?.selectedText ?: return@registerAction

    val client = java.net.Socket("127.0.0.1", 9999)

    val actionGroup = PopupActionGroup("Actions with the regexp",
        AnAction("Compile") {
            val url = java.net.URL("https://reqres.in/api/users?page=1")
            val connection = url.openConnection()
            java.io.BufferedReader(java.io.InputStreamReader(connection.getInputStream())).use { inp ->
                var line: String?
                while (inp.readLine().also { line = it } != null) {
                    println(line)
                }
            }
            Messages.showInfoMessage("It's ok!", "Result")
        },
        AnAction("Test") {
            while(true) {
                val command = Messages.showInputDialog("Enter a test string:", "Testing", null)
                if (command.isNullOrBlank()) break
                if (command != null) Messages.showInfoMessage(command, "Result")
            }
        },
        Separator.getInstance(),
        AnAction("Explanation") { popupEvent ->
            Messages.showInfoMessage("I don't know how it work", "Result")
        },
        AnAction("Edits") {
            Messages.showInfoMessage("I don't know what to say", "Result")
        },
        PopupActionGroup("Hints",
            AnAction("Search command by description") {
                val command = Messages.showInputDialog("Enter a description:", "Search", null)
                if (command != null) Messages.showInfoMessage("I don't know how it work", "Result")
            },
            AnAction("Search description by command") {
                val command = Messages.showInputDialog("Enter a command:", "Search", null)
                if (command != null) Messages.showInfoMessage("I don't know how it work", "Result")
            }
        )
    )
    actionGroup.createPopup(event.dataContext).showCenteredInCurrentWindow(event.project!!)
    client.close()
}

if (!isIdeStartup) show("Loaded 'Show Actions Popup'<br/>Use ctrl+shift+H to run it")

import com.intellij.openapi.actionSystem.AnActionEvent
import com.intellij.openapi.actionSystem.Separator
import com.intellij.openapi.ui.Messages
import liveplugin.*
import liveplugin.PluginUtil.currentEditorIn
import kotlin.concurrent.thread
import java.io.BufferedReader
import java.io.InputStreamReader
import java.util.stream.Collectors

registerAction(id = "Show Actions RWP", keyStroke = "ctrl shift H") { event: AnActionEvent ->
    val editor = event.project?.let { it1 -> currentEditorIn(it1) }
    val text = editor?.selectionModel?.selectedText ?: return@registerAction

    val actionGroup = PopupActionGroup("Actions with the regexp",
        AnAction("Check for errors") {
            Messages.showInfoMessage(run(arrayOf("wr22-client", "parse", text)), "Result")
        },
        AnAction("Test a string") {
            while(true) {
                val command = Messages.showInputDialog("Enter a test string:", "Testing", null)
                if (command.isNullOrBlank()) break
                if (command != null) {
                    Messages.showInfoMessage(run(arrayOf("wr22-client", "match", text, command)), "Result")
                }
            }
        },
        Separator.getInstance(),
        AnAction("Explain") { popupEvent ->
            Messages.showInfoMessage(run(arrayOf("wr22-client", "explain", text)), "Result")
        },
    )
    actionGroup.createPopup(event.dataContext).showCenteredInCurrentWindow(event.project!!)
}

fun run(args: Array<String>): String {
    val builder = ProcessBuilder("/usr/bin/env", *args);
    try {
        val process = builder.start()
        val stream = process.getInputStream()
        process.waitFor()
        val output = BufferedReader(InputStreamReader(stream)).lines().collect(Collectors.joining(" "))
        val exitCode = process.exitValue()
        if (exitCode != 0) {
            show("Process exited with error code ${exitCode}")
        }
        return output
    } catch (e: Exception) {
        show("Failed to execute, ${e}")
    }
    return "";
}

thread {
    val builder = ProcessBuilder("/usr/bin/env", "wr22-regex-server")
    try {
        val process = builder.start()
        process.waitFor()
        val exitCode = process.exitValue()
        if (exitCode != 0) {
            show("Server exited with error code ${exitCode}")
        }
    } catch (e: Exception) {
        show("Failed to execute server, ${e}")
    }
}

show("Loaded 'Show Actions Popup'<br/>Use ctrl+shift+H to run it")

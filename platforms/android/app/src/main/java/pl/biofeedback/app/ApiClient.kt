package pl.biofeedback.app

import org.json.JSONObject
import java.io.IOException
import java.net.HttpURLConnection
import java.net.URL

/**
 * HTTPS-only API client. Call these blocking methods from Dispatchers.IO or
 * another worker thread; never from the Compose main thread.
 * The bearer token is held only in process memory.
 */
class ApiClient(baseUrl: String) {
    private val endpoint = baseUrl.trimEnd('/')
    @Volatile private var accessToken: String? = null

    init {
        require(endpoint.startsWith("https://", ignoreCase = true)) {
            "API endpoint must use HTTPS; cleartext network access is disabled."
        }
    }

    @Synchronized
    fun login(username: String, password: String): JSONObject {
        val payload = JSONObject()
            .put("username", username)
            .put("password", password)
        val response = request("POST", "/api/v1/auth/login", payload, authenticated = false)
        val token = response.optString("access_token")
        if (token.length != 64) throw ApiException(502, "API returned an invalid access token.")
        accessToken = token
        return response
    }

    fun currentUser(): JSONObject =
        request("GET", "/api/v1/auth/me", null, authenticated = true)

    @Synchronized
    fun logout() {
        try {
            request("POST", "/api/v1/auth/logout", JSONObject(), authenticated = true)
        } finally {
            accessToken = null
        }
    }

    private fun request(
        method: String,
        path: String,
        body: JSONObject?,
        authenticated: Boolean
    ): JSONObject {
        val connection = (URL(endpoint + path).openConnection() as HttpURLConnection).apply {
            requestMethod = method
            connectTimeout = 5000
            readTimeout = 5000
            setRequestProperty("Accept", "application/json")
            setRequestProperty("Cache-Control", "no-store")
            if (authenticated) {
                val token = accessToken ?: throw ApiException(401, "Authentication required.")
                setRequestProperty("Authorization", "Bearer $token")
            }
            if (body != null) {
                doOutput = true
                setRequestProperty("Content-Type", "application/json; charset=utf-8")
            }
        }

        try {
            if (body != null) {
                connection.outputStream.use { output ->
                    output.write(body.toString().toByteArray(Charsets.UTF_8))
                }
            }
            val status = connection.responseCode
            val stream = if (status in 200..299) connection.inputStream else connection.errorStream
            val raw = stream?.bufferedReader(Charsets.UTF_8)?.use { it.readText() }.orEmpty()
            val payload = if (raw.isBlank()) JSONObject() else JSONObject(raw)
            if (status !in 200..299) {
                throw ApiException(status, payload.optString("message", "API request failed."))
            }
            return payload
        } catch (error: ApiException) {
            throw error
        } catch (error: IOException) {
            throw ApiException(0, "API is unavailable: ${error.message ?: "network error"}")
        } finally {
            connection.disconnect()
        }
    }
}

class ApiException(val status: Int, message: String) : Exception(message)

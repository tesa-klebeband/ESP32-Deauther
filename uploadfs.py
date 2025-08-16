Import("env")
# uplaod /data to SPIFFS
# This is used to upload files to the SPIFFS filesystem on the ESP32
def after_upload(source, target, env):
    env.Execute("pio run --target uploadfs --environment " + env['PIOENV'])
env.AddPostAction("upload", after_upload)

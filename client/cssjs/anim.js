let body = document.getElementById("bodyduh") // i hate this implementation
let circle = document.getElementById("circle") // ok for this it's not bad
let debugh1 = document.getElementById("debug")
document.addEventListener("DOMContentLoaded", () => {
    document.documentElement.style.setProperty("--screenWidth", window.screen.width + "px")
    document.documentElement.style.setProperty("--screenHeight", window.screen.height + "px")
    setTimeout(hideCircle, 4900)
})

function hideCircle() {
    circle.remove()
    body.style.backgroundColor = "black"
    body.style.overflow = "auto"
}
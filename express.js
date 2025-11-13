const express = require('express');
const app = express();
const port = 3000;

app.use('/cssjs', express.static(__dirname + '/client/cssjs'));

app.get('/', (req, res) => {
  res.sendFile(__dirname + '/client/index.html');
});


app.listen(port, () => {
  console.log(`Server is running at http://localhost:${port}`);
});
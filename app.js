const express = require("express");
const mongoose =require("mongoose");
const bodyparser = require("body-parser")
const client = require('./models/client.js')
var exphbs  = require('express-handlebars');
var fs = require("fs");
const app = express();
let reduction=2000;
//MIDDLEWARE To parse incomming request
app.use(bodyparser());
app.use(bodyparser.urlencoded({ extended: false }))
//app.use(express.json());
//app.use(express.urlencoded({extended: false}));
const PORT = 4005;
//connnect to db
mongoose.connect('mongodb://localhost:27017/RESTAURENT',{ useUnifiedTopology: true, useNewUrlParser: true })
.then(()=>
console.log('connected to mongodb locally')
)
.catch((error) =>{
console.log("not connected to db");
console.error(error)
}
)

//handlebars template engine 
app.engine('handlebars', exphbs());
app.set('view engine', 'handlebars');

//handle get request
app.get("/", async (req,res)=>{
 try {
    
      const results = await client.find().lean().limit(15);
         res.render('home',{
         results,
      })
  } catch (error) {
    console.log(error);
    res.json(error);
  }
})

app.get('/recharge',(req,res)=>{
   fs.readFile("card.txt", function (err, buf) {
  console.log(buf.toString());
  let card= buf.toString();
    let errors ="";
    res.render("recharge",{
    card 
    })
})
})

//render recharge template 
app.get('/add',(req,res)=>{
 fs.readFile("card.txt", function (err, buf) {
  console.log(buf.toString());
  let card= buf.toString();
    let errors ="";
    res.render("add",{
    card 
    })
})


})

//payement of food // reduction of 2000frw on each card placacement
app.post("/recharge", async (req,res)=>{
    let {card,balance} = req.body;
    if(!card){
        return res.render('add',{
            error:"no card number entered. fill it "
        })
    }
     if(!balance){
        return res.render('add',{
            error:"no balance filled in. fill it please!! "
        })
    }
    try {
        const user = await client.findOne({card:card})
   if(!user){
       console.log(`no user with :${user.card} found`)
       return res.render("add",{
           error:`no user with :${user.card} found on our server`
       })
   }

    let newBalance = parseInt(user.balance) + parseInt(balance);
    const savedBalance = await client.findOneAndUpdate({card:card},{balance:newBalance})
    //const savedBalance = await client.updateOne({balance:newBalance})
    fs.truncate("card.txt",(err,buff)=>{
    console.log("delete card done");
})
    res.render("add",{
        message:`User:${savedBalance.firstname}, Your balance ${newBalance}`
    })
    } catch (error) {
        console.log(error)
    }
})

//payement of food // reduction of 2000frw on each card placacement
app.get("/card", async (req,res)=>{
    let {rfid,sn} = req.query;
    const data = rfid
    try {
//delete file content if exits
fs.truncate("card.txt",(err,buff)=>{
    console.log("done");
})
//write new card number in the file 
 fs.writeFile("card.txt", data, (err) => {
  if (err) console.log(err);
  console.log("Successfully Written to File.");
});

   const user = await client.findOne({card:rfid})
   if(!user){
       console.log(`no user with :${rfid} found`)
       return res.send(`no user with :${rfid} found on our server`)
   }
   
        if(user.balance < reduction){
       console.log(`Insufficient Balance.Only:${user.balance} available`)
       return res.send(`Insufficient Balance.Only:${user.balance} available`)
   }
    let newBalance = user.balance-reduction;
    const savedBalance = await client.findOneAndUpdate({card:rfid},{balance:newBalance})
    res.send({newbalance:newBalance})
    } catch (error) {
        console.log(error)
    }
})




app.post("/save", async (req,res)=>{
    console.log(req.body);
    let {card,firstname,lastname,email} = req.body;
     try {
      const user = await client.findOne({card:card})
   if(user){
       console.log(` user with :${user.card} Already exists`)
       return res.render('recharge',{
           error:`user with :${user.card} Already exists`
       })
   }
   
        const newuser = new client({
        firstname:firstname,
        lastname:lastname,
        email:email,
        balance:"0",
        card:card
    })

    const savedUser = await newuser.save()
    res.render("recharge",{
        message:"User Created"
    })
    
     } catch (error) {
         
     }
})

app.use('*', (req,res)=>{
    res.send("404....resource not exist on server")
})



app.listen(PORT,()=> console.log("app is running on port:" + PORT));

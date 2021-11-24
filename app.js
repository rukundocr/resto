const express = require("express");
const mongoose =require("mongoose");
const bodyparser = require("body-parser")
const client = require('./models/client.js')
const transaction = require('./models/transaction.js')
var exphbs  = require('express-handlebars');
const Pusher = require("pusher");
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
mongoose.connect('mongodb://localhost:27017/PRAYERS',{ useUnifiedTopology: true, useNewUrlParser: true })
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
app.set('view engine', 'handlebars')

//pusher 
const pusher = new Pusher({
  appId: "1301327",
  key: "c869dddafabe66adaf63",
  secret: "6c7cbd57197fe35afad1",
  cluster: "mt1",
  useTLS: true
});



//handle get request
app.get("/", async (req,res)=>{
 try {
    
      const results = await client.find().lean().limit(100);
         res.render('home',{
         results,
      })
  } catch (error) {
    console.log(error);
    res.json(error);
  }
})

//get normal prayers 
app.get("/normal", async (req,res)=>{
 try {
     const status="Normal"
      const results = await transaction.find({status:status}).lean().limit(100);
      results.reverse();
         res.render('normal',{
         results,
      })
  } catch (error) {
    console.log(error);
    res.json(error);
  }
})

app.get("/abnormal", async (req,res)=>{
 try {
     const status="Abnormal"
      const results = await transaction.find({status:status}).lean().limit(100);
      results.reverse();
         res.render('abnormal',{
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
app.get("/card", async (req,res)=>{
    console.log(req.query);
    let {rfid,temp} = req.query;
    const data = rfid
    const temperature = parseFloat(temp);
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
       pusher.trigger("missing", "notFound", {
       message:`GET REGISTERED FIRST AND TRY AGAIN !!`,
       temperature:temperature
});
       return res.json({
         status:"Unregistered",
         temp:temperature  
       })
       
   }
    const today = new Date();   
    const  time = today.getFullYear()+'-'+(today.getMonth()+1)+'-'+today.getDate() + '-'+ today.getHours() + ":" + today.getMinutes() + ":" + today.getSeconds();
     if(temperature>37){
         let NewRecord = new transaction({
             firstname:user.firstname,
             lastname:user.lastname,
             NID:user.NID,
             District:user.District,
             Sector:user.Sector,
             Cell:user.Cell,
             Village:user.Village,
             Phone:user.Phone,
             temperature:temperature ,
             status:"Abnormal" ,
             card:user.card,
             time:time
         })
          pusher.trigger("mirembe", "my-event", {
             firstname:user.firstname,
             lastname:user.lastname,
             NID:user.NID,
             temperature:temperature ,
             status:"Abnormal" ,
             card:user.card,
             id:user._id
});
         const savedUser = await NewRecord.save()
         return res.json({
             firstname:user.firstname,
             lastname:user.lastname,
             status:"Abnormal",
             temp:temperature
         })
     }
    
    if(temperature<37){
        
         let NewRecord = new transaction({
             firstname:user.firstname,
             lastname:user.lastname,
             NID:user.NID,
             District:user.District,
             Sector:user.Sector,
             Cell:user.Cell,
             Village:user.Village,
             Phone:user.Phone,
             temperature:temperature ,
             status:"Normal" ,
             card:user.card,
             time:time
         })
            pusher.trigger("mirembe", "my-event", {
             firstname:user.firstname,
             lastname:user.lastname,
             NID:user.NID,
             temperature:temperature ,
             status:"Normal" ,
             card:user.card,
             id:user._id
});
         const savedUser = await NewRecord.save()
         return res.json({
             firstname:user.firstname,
             lastname:user.lastname,
             status:"Normal",
             temp:temperature
         })
     }

      
    } catch (error) {
        console.log(error)
    }
})



// Register new user 
app.post("/save", async (req,res)=>{
    console.log(req.body);
    let {card,firstname,lastname,NID,District,Sector,Cell,Village,Phone} = req.body;
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
        NID:NID,
        District:District,
        Sector:Sector,
        Cell:Cell,
        Village:Village,
        Phone:Phone,
        card:card
    })

    const savedUser = await newuser.save()
    res.render("recharge",{
        message:"User Created"
    })
    
     } catch (error) {
         
     }
})

//delete abnormal records

app.get("/abnormal/delete/:id", async (req,res)=>{
    let {id}= req.params
    console.log(id);
    try {
        let deletedRecord = await transaction.findByIdAndDelete({_id:id})
        const status="Abnormal"
        const results = await transaction.find({status:status}).lean().limit(15);
         res.render('abnormal',{
            message:"Record Deleted",
            results
        });
        
    } catch (error) {
        console.log(error)
        res.send('error occured while deleting  ......')
    }

})

//delete normal records

app.get("/normal/delete/:id", async (req,res)=>{
    let {id}= req.params
    console.log(id);
    try {
        let deletedRecord = await transaction.findByIdAndDelete({_id:id})
        const status="Normal"
        const results = await transaction.find({status:status}).lean().limit(15);
        return res.render('normal',{
            message:"Record Deleted",
            results
        });
        
    } catch (error) {
        console.log(error)
        res.send('error occured while deleting  ......')
    }

})

//live attendance
app.get('/live',(req,res)=>{
    res.render("live");
})

// user details
app.get('/details', async  (req,res)=>{
    let card ="";
    fs.readFile("card.txt", async function (err, buf) {
  //console.log(buf.toString());
    card= buf.toString();
     try {
         const results = await transaction.find({card:card}).lean().limit(30)
         console.log(results);
         res.render('details',{
             results
         })
     } catch (error) {
         console.log(error)
     }
})
 
})


app.use('*', (req,res)=>{
    res.send("404....resource not exist on server")
})



app.listen(PORT,()=> console.log("app is running on port:" + PORT));
